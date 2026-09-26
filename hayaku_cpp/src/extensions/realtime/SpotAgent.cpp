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

#include "common/Lang.h"
#include "extensions/realtime/RealtimePort.h"
#include "extensions/realtime/spot_generated.h"

using namespace hayaku::flat;

namespace hayaku {

namespace {
thread_local const SpotAgent* g_callbackAgent = nullptr;

class CallbackScope {
 public:
  explicit CallbackScope(const SpotAgent* agent) : previous_(g_callbackAgent) {
    g_callbackAgent = agent;
    detail::enterRealtimeCallback();
  }

  ~CallbackScope() {
    detail::leaveRealtimeCallback();
    g_callbackAgent = previous_;
  }

 private:
  const SpotAgent* previous_;
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
  HAYAKU_CHECK(work_num_ > 0,
               "SpotAgent worker count must be greater than zero");
  HAYAKU_INFO(htr("Start spot agent."));
  stop();

  std::lock_guard<std::mutex> lock(run_mutex_);
  if (stop_) {
    try {
      receive_data_tg_ = std::make_unique<ThreadPool>(1);
      tg_ = std::make_unique<ThreadPool>(work_num_);
      cleanup_pending_.store(true, std::memory_order_release);
      stop_ = false;
      receive_thread_ = std::thread([this]() { work_thread(); });
    } catch (...) {
      stop_ = true;
      tg_.reset();
      receive_data_tg_.reset();
      cleanup_pending_.store(false, std::memory_order_release);
      throw;
    }
  }
}

void SpotAgent::stop() {
  // A callback runs inside one of the pools below. It may request shutdown, but
  // joining that pool here would wait for this very callback to return.
  if (isInCallback()) {
    stop_ = true;
    return;
  }

  std::lock_guard<std::mutex> lock(run_mutex_);
  stop_ = true;
  if (receive_thread_.joinable()) {
    receive_thread_.join();
  }
  if (receive_data_tg_) {
    receive_data_tg_->join();
  }
  if (tg_) {
    tg_->join();
  }
  if (receive_data_tg_) {
    receive_data_tg_.reset();
  }
  if (tg_) {
    tg_.reset();
  }
  connected_ = false;
  status_ = WAITING;
  cleanup_pending_.store(false, std::memory_order_release);
}

bool SpotAgent::isInCallback() const noexcept {
  return g_callbackAgent == this;
}

class ProcessTask {
 public:
  ProcessTask(SpotAgent* agent,
              const std::function<void(const SpotRecord&)>& func,
              const SpotRecord& spot)
      : agent_(agent), func_(func), spot_(spot) {}

  void operator()() {
    CallbackScope scope(agent_);
    try {
      func_(spot_);
    } catch (const std::exception& e) {
      HAYAKU_ERROR(e.what());
    } catch (...) {
      HAYAKU_ERROR_UNKNOWN;
    }
  }

 private:
  SpotAgent* agent_;
  std::function<void(const SpotRecord&)> func_;
  SpotRecord spot_;
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
  HAYAKU_CHECK(VerifySpotListBuffer(verify), "Invalid data!");

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4267)
#endif

  // Update the K-line data
  auto* spot_list = GetSpotList(spot_list_buf);
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
      for (const auto& process : process_list_) {
        tasks.emplace_back(
            tg_->submit(ProcessTask(this, process, *spot_record)));
      }
    }
  }

  for (auto& task : tasks) {
    task.get();
  }
  HAYAKU_DEBUG("received count: {}", total);
  for (const auto& postProcess : post_process_list_) {
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
    stop_ = true;
    return;
  }

  rv = nng_socket_set(sock, NNG_OPT_SUB_SUBSCRIBE, ms_spotTopic,
                      ms_spotTopicLength);
  if (rv != 0) {
    HAYAKU_ERROR("Failed set nng socket option! {}", nng_strerror(rv));
    nng_close(sock);
    stop_ = true;
    return;
  }

  rv = nng_socket_set_ms(sock, NNG_OPT_RECVTIMEO, rev_timeout_);
  if (rv != 0) {
    HAYAKU_ERROR("Failed set receive timeout option! {}", nng_strerror(rv));
    nng_close(sock);
    stop_ = true;
    return;
  }

  rv = -1;
  Datetime pretime = Datetime::now();
  Datetime startReceiveTime;
  while (!stop_ && rv != 0) {
    rv = nng_dial(sock, ms_pubUrl.c_str(), nullptr, 0);
    auto now = Datetime::now();
    HAYAKU_WARN_IF(
        print_ && rv != 0 && (now - pretime) > Seconds(5),
        "Faied connect quotation server {}, will retry after 5 seconds!",
        ms_pubUrl);
    pretime = now;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }

  HAYAKU_INFO_IF(!stop_ && print_, "Ready to receive quotation from {} ...",
                 ms_pubUrl);

  if (stop_ || rv != 0) {
    nng_close(sock);
    return;
  }

  connected_ = true;
  while (!stop_) {
    char* buf = nullptr;
    size_t length = 0;
    try {
      rv = nng_recv(sock, &buf, &length, NNG_FLAG_ALLOC);
      if (rv != 0 && rv != NNG_ETIMEDOUT) {
        HAYAKU_ERROR("Failed nng_recv! {}", nng_strerror(rv));
        if (buf) {
          nng_free(buf, length);
        }
        stop_ = true;
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
      switch (status_) {
        case WAITING:
          if (length == ms_startTagLength &&
              memcmp(buf, ms_startTag, ms_startTagLength) == 0) {
            startReceiveTime = Datetime::now();
            status_ = RECEIVING;
          }
          break;
        case RECEIVING:
          if (length == ms_endTagLength &&
              memcmp(buf, ms_endTag, ms_endTagLength) == 0) {
            status_ = WAITING;
          } else if (length != ms_startTagLength ||
                     memcmp(buf, ms_startTag, ms_startTagLength) != 0) {
            std::shared_ptr<char[]> data_buf(new char[length]);
            memcpy(data_buf.get(), buf, length);
            receive_data_tg_->submit([this, length, startReceiveTime,
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

  connected_ = false;
  nng_close(sock);
}

void SpotAgent::addProcess(std::function<void(const SpotRecord&)> process) {
  HAYAKU_CHECK(stop_ && !cleanup_pending_.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> runLock(run_mutex_);
  HAYAKU_CHECK(stop_ && !cleanup_pending_.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> lock(mutex_);
  process_list_.push_back(process);
}

void SpotAgent::addPostProcess(std::function<void(Datetime)> func) {
  HAYAKU_CHECK(stop_ && !cleanup_pending_.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> runLock(run_mutex_);
  HAYAKU_CHECK(stop_ && !cleanup_pending_.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> lock(mutex_);
  post_process_list_.push_back(func);
}

void SpotAgent::clearProcessList() {
  HAYAKU_CHECK(stop_ && !cleanup_pending_.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> runLock(run_mutex_);
  HAYAKU_CHECK(stop_ && !cleanup_pending_.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> lock(mutex_);
  process_list_.clear();
}

void SpotAgent::clearPostProcessList() {
  HAYAKU_CHECK(stop_ && !cleanup_pending_.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> runLock(run_mutex_);
  HAYAKU_CHECK(stop_ && !cleanup_pending_.load(std::memory_order_acquire),
               "SpotAgent has active workers, please stop agent first!");
  std::lock_guard<std::mutex> lock(mutex_);
  post_process_list_.clear();
}

}  // namespace hayaku
