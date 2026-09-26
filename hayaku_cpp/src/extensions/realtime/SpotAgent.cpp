/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-20
 *     Author: fasiondog
 */

#include "SpotAgent.h"

#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>

#include <chrono>
#include <utility>

#include "common/Lang.h"
#include "extensions/realtime/RealtimePort.h"
#include "extensions/realtime/spot_generated.h"

namespace hayaku {

namespace {
thread_local const SpotAgent* g_callbackAgent = nullptr;

class CallbackScope {
 public:
  explicit CallbackScope(const SpotAgent* agent) : m_previous(g_callbackAgent) {
    g_callbackAgent = agent;
    detail::enterRealtimeCallback();
  }

  ~CallbackScope() {
    detail::leaveRealtimeCallback();
    g_callbackAgent = m_previous;
  }

 private:
  const SpotAgent* m_previous;
};
}  // namespace

string SpotAgent::ms_pubUrl{
    "ipc:///tmp/hayaku_real.ipc"};  // The data sending service address
const char* SpotAgent::ms_startTag = ":spot:[start spot]";
const char* SpotAgent::ms_endTag = ":spot:[end spot]";
const char* SpotAgent::ms_spotTopic = ":spot:";
const size_t SpotAgent::ms_spotTopicLength = strlen(SpotAgent::ms_spotTopic);
const size_t SpotAgent::ms_startTagLength = strlen(SpotAgent::ms_startTag);
const size_t SpotAgent::ms_endTagLength = strlen(SpotAgent::ms_endTag);

SpotAgent::~SpotAgent() { stop(); }

void SpotAgent::setQuotationServer(const string& server) { ms_pubUrl = server; }

void SpotAgent::start() {
  HAYAKU_CHECK(!isInCallback(),
               "Cannot restart SpotAgent from its own callback");
  HAYAKU_CHECK(m_work_num > 0,
               "SpotAgent worker count must be greater than zero");
  HAYAKU_INFO(htr("Start spot agent."));
  stop();

  std::lock_guard<std::mutex> lock(m_run_mutex);
  if (m_stop) {
    try {
      m_receive_data_tg = std::make_unique<ThreadPool>(1);
      m_tg = std::make_unique<ThreadPool>(m_work_num);
      m_cleanupPending.store(true, std::memory_order_release);
      m_stop = false;
      m_receiveThread = std::thread([this]() { work_thread(); });
    } catch (...) {
      m_stop = true;
      m_tg.reset();
      m_receive_data_tg.reset();
      m_cleanupPending.store(false, std::memory_order_release);
      throw;
    }
  }
}

void SpotAgent::stop() {
  // A callback runs inside one of the pools below. It may request shutdown, but
  // joining that pool here would wait for this very callback to return.
  if (isInCallback()) {
    m_stop = true;
    return;
  }

  std::lock_guard<std::mutex> lock(m_run_mutex);
  m_stop = true;
  if (m_receiveThread.joinable()) {
    m_receiveThread.join();
  }
  if (m_receive_data_tg) {
    m_receive_data_tg->join();
  }
  if (m_tg) {
    m_tg->join();
  }
  if (m_receive_data_tg) {
    m_receive_data_tg.reset();
  }
  if (m_tg) {
    m_tg.reset();
  }
  m_connected = false;
  m_status = WAITING;
  m_cleanupPending.store(false, std::memory_order_release);
}

bool SpotAgent::isInCallback() const noexcept {
  return g_callbackAgent == this;
}

class ProcessTask {
 public:
  ProcessTask(SpotAgent* agent,
              const std::function<void(const SpotRecord&)>& func,
              const SpotRecord& spot)
      : m_agent(agent), m_func(func), m_spot(spot) {}

  void operator()() {
    CallbackScope scope(m_agent);
    try {
      m_func(m_spot);
    } catch (const std::exception& e) {
      HAYAKU_ERROR(e.what());
    } catch (...) {
      HAYAKU_ERROR_UNKNOWN;
    }
  }

 private:
  SpotAgent* m_agent;
  std::function<void(const SpotRecord&)> m_func;
  SpotRecord m_spot;
};

unique_ptr<SpotRecord> SpotAgent::parseFlatSpot(
    const hayaku::flat::Spot* spot) {
  if (!spot || !spot->datetime()) {
    HAYAKU_WARN("Ignore spot record without datetime");
    return nullptr;
  }

  try {
    auto result = std::make_unique<SpotRecord>();
    if (spot->market()) result->market = spot->market()->str();
    if (spot->code()) result->code = spot->code()->str();
    if (spot->name()) result->name = spot->name()->str();
    result->datetime = Datetime(spot->datetime()->str());
    result->yesterday_close = spot->yesterday_close();
    result->open = spot->open();
    result->high = spot->high();
    result->low = spot->low();
    result->close = spot->close();
    result->amount = spot->amount();
    result->volume = spot->volume();
    auto* bids = spot->bid();
    if (bids) {
      size_t length = bids->size();
      result->bid.resize(length);
      for (size_t i = 0; i < length; i++) {
        result->bid[i] = bids->Get(i);
      }
    }

    auto* bid_amounts = spot->bid_amount();
    if (bid_amounts) {
      size_t length = bid_amounts->size();
      result->bid_amount.resize(length);
      for (size_t i = 0; i < length; i++) {
        result->bid_amount[i] = bid_amounts->Get(i);
      }
    }

    auto* asks = spot->ask();
    if (asks) {
      size_t length = asks->size();
      result->ask.resize(length);
      for (size_t i = 0; i < length; i++) {
        result->ask[i] = asks->Get(i);
      }
    }

    auto* asks_amounts = spot->ask_amount();
    if (asks_amounts) {
      size_t length = asks_amounts->size();
      result->ask_amount.resize(length);
      for (size_t i = 0; i < length; i++) {
        result->ask_amount[i] = asks_amounts->Get(i);
      }
    }

    return result;
  } catch (const std::exception& e) {
    HAYAKU_ERROR(e.what());
  } catch (...) {
    HAYAKU_ERROR_UNKNOWN;
  }

  return nullptr;
}

void SpotAgent::parseSpotData(const void* buf, size_t buf_len,
                              const Datetime& startReceiveTime) {
  // SPEND_TIME(receive_data);
  HAYAKU_CHECK(buf && buf_len > ms_spotTopicLength &&
                   memcmp(buf, ms_spotTopic, ms_spotTopicLength) == 0,
               "Invalid spot message!");
  const auto* spot_list_buf =
      static_cast<const uint8_t*>(buf) + ms_spotTopicLength;

  // Validate the data
  flatbuffers::Verifier verify(spot_list_buf, buf_len - ms_spotTopicLength);
  HAYAKU_CHECK(flat::VerifySpotListBuffer(verify), "Invalid data!");

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4267)
#endif

  // Update the K-line data
  auto* spot_list = flat::GetSpotList(spot_list_buf);
  auto* spots = spot_list->spot();
  if (!spots) {
    return;
  }
  size_t total = spots->size();
  vector<std::future<void>> tasks;
  for (size_t i = 0; i < total; i++) {
    auto* spot = spots->Get(i);
    auto spot_record = parseFlatSpot(spot);
    if (spot_record) {
      for (const auto& process : m_processList) {
        tasks.emplace_back(
            m_tg->submit(ProcessTask(this, process, *spot_record)));
      }
    }
  }

  for (auto& task : tasks) {
    task.get();
  }
  HAYAKU_DEBUG("received count: {}", total);
  for (const auto& postProcess : m_postProcessList) {
    CallbackScope scope(this);
    postProcess(startReceiveTime);
  }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}

void SpotAgent::work_thread() {
  nng_socket sock;

  int rv = nng_sub0_open(&sock);
  if (rv != 0) {
    HAYAKU_ERROR("Can't open nng sub0! {}", nng_strerror(rv));
    m_stop = true;
    return;
  }

  rv = nng_socket_set(sock, NNG_OPT_SUB_SUBSCRIBE, ms_spotTopic,
                      ms_spotTopicLength);
  if (rv != 0) {
    HAYAKU_ERROR("Failed set nng socket option! {}", nng_strerror(rv));
    nng_close(sock);
    m_stop = true;
    return;
  }

  rv = nng_socket_set_ms(sock, NNG_OPT_RECVTIMEO, m_revTimeout);
  if (rv != 0) {
    HAYAKU_ERROR("Failed set receive timeout option! {}", nng_strerror(rv));
    nng_close(sock);
    m_stop = true;
    return;
  }

  rv = -1;
  Datetime pretime = Datetime::now();
  Datetime startReceiveTime;
  while (!m_stop && rv != 0) {
    rv = nng_dial(sock, ms_pubUrl.c_str(), nullptr, 0);
    auto now = Datetime::now();
    HAYAKU_WARN_IF(
        m_print && rv != 0 && (now - pretime) > Seconds(5),
        "Faied connect quotation server {}, will retry after 5 seconds!",
        ms_pubUrl);
    pretime = now;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }

  HAYAKU_INFO_IF(!m_stop && m_print, "Ready to receive quotation from {} ...",
                 ms_pubUrl);

  if (m_stop || rv != 0) {
    nng_close(sock);
    return;
  }

  m_connected = true;
  while (!m_stop) {
    char* buf = nullptr;
    size_t length = 0;
    try {
      rv = nng_recv(sock, static_cast<void*>(&buf), &length, NNG_FLAG_ALLOC);
      if (rv != 0 && rv != NNG_ETIMEDOUT) {
        HAYAKU_ERROR("Failed nng_recv! {}", nng_strerror(rv));
        if (buf) {
          nng_free(buf, length);
        }
        m_stop = true;
        break;
      }
      if (rv == NNG_ETIMEDOUT) {
        if (buf) {
          nng_free(buf, length);
        }
        continue;
      }
      if (!buf || length == 0) {
        if (buf) {
          nng_free(buf, length);
        }
        continue;
      }
      switch (m_status) {
        case WAITING:
          if (length == ms_startTagLength &&
              memcmp(buf, ms_startTag, ms_startTagLength) == 0) {
            startReceiveTime = Datetime::now();
            m_status = RECEIVING;
          }
          break;
        case RECEIVING:
          if (length == ms_endTagLength &&
              memcmp(buf, ms_endTag, ms_endTagLength) == 0) {
            m_status = WAITING;
          } else if (length != ms_startTagLength ||
                     memcmp(buf, ms_startTag, ms_startTagLength) != 0) {
            std::shared_ptr<char[]> data_buf(new char[length]);
            memcpy(data_buf.get(), buf, length);
            m_receive_data_tg->submit([this, length, startReceiveTime,
                                       new_buf = std::move(data_buf)]() {
              try {
                this->parseSpotData(new_buf.get(), length, startReceiveTime);
              } catch (const std::exception& e) {
                HAYAKU_ERROR(e.what());
              } catch (...) {
                HAYAKU_ERROR_UNKNOWN;
              }
            });
          }  // else {keep waiting for the data}
          break;
      }
    } catch (std::exception& e) {
      HAYAKU_ERROR(e.what());
    } catch (...) {
      HAYAKU_ERROR_UNKNOWN;
    }
    if (buf) {
      nng_free(buf, length);
    }
  }

  m_connected = false;
  nng_close(sock);
}

void SpotAgent::addProcess(std::function<void(const SpotRecord&)> process) {
  HAYAKU_CHECK(m_stop && !m_cleanupPending.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> runLock(m_run_mutex);
  HAYAKU_CHECK(m_stop && !m_cleanupPending.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> lock(m_mutex);
  m_processList.push_back(std::move(process));
}

void SpotAgent::addPostProcess(std::function<void(Datetime)> func) {
  HAYAKU_CHECK(m_stop && !m_cleanupPending.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> runLock(m_run_mutex);
  HAYAKU_CHECK(m_stop && !m_cleanupPending.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> lock(m_mutex);
  m_postProcessList.push_back(std::move(func));
}

void SpotAgent::clearProcessList() {
  HAYAKU_CHECK(m_stop && !m_cleanupPending.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> runLock(m_run_mutex);
  HAYAKU_CHECK(m_stop && !m_cleanupPending.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> lock(m_mutex);
  m_processList.clear();
}

void SpotAgent::clearPostProcessList() {
  HAYAKU_CHECK(m_stop && !m_cleanupPending.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> runLock(m_run_mutex);
  HAYAKU_CHECK(m_stop && !m_cleanupPending.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> lock(m_mutex);
  m_postProcessList.clear();
}

}  // namespace hayaku
