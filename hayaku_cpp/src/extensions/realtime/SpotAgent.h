#pragma once

/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-20
 *     Author: fasiondog
 */

#include <functional>
#include <thread>

#include "common/concurrency/ThreadPool.h"
#include "data/MarketTypes.h"
#include "extensions/realtime/RealtimeExport.h"
#include "extensions/realtime/SpotRecord.h"

namespace hayaku {
namespace flat {
struct Spot;
}
}  // namespace hayaku

namespace hayaku {

/**
 * Agent that receives the external realtime data
 * @ingroup Agent
 */
class HAYAKU_REALTIME_API SpotAgent {
 public:
  SpotAgent() = default;

  /** Destructor */
  virtual ~SpotAgent();

  /** Start the agent */
  void start();

  /** Stop the agent */
  void stop();

  /** Whether it is running */
  bool isRunning() const noexcept { return !stop_; }

  void setWorkerNum(size_t worker_num) { work_num_ = worker_num; }

  size_t getWorkerNum() const { return work_num_; }

  /** Set whether to print the data receiving progress; it is mainly used to
   * turn off the printing in an interactive environment */
  void setPrintFlag(bool print) { print_ = print; }

  bool getPrintFlag() const { return print_; }

  void setServerAddr(const string& addr) { server_addr_ = addr; }

  const string& getServerAddr() const { return server_addr_; }

  bool isConnected() const { return connected_; }

  /**
   * Add the handler called when Spot data is received
   * @note This operation can only be performed in the stopped state, otherwise
   * an exception is thrown
   * @param process the handler, it processes a single spot record only
   */
  void addProcess(std::function<void(const SpotRecord&)> process);

  /**
   * Add the post-processing function to be called after all the batches of data
   * at a certain moment have been received; the given datetime is the moment
   * when the data is received (not the time when the data has been fully
   * processed)
   * @note This operation can only be performed in the stopped state, otherwise
   * an exception is thrown
   * @param func the post-processing function
   */
  void addPostProcess(std::function<void(Datetime)> func);

  /**
   * Clear all the handlers added before
   * @note This operation can only be performed in the stopped state, otherwise
   * an exception is thrown
   */
  void clearProcessList();

  /**
   * Clear all the post-processing functions added before
   * @note This operation can only be performed in the stopped state, otherwise
   * an exception is thrown
   */
  void clearPostProcessList();

 public:
  static void setQuotationServer(const string& server);

 private:
  friend SpotAgent* getGlobalSpotAgent();
  friend void releaseGlobalSpotAgent();
  friend bool realtimePortQuiescent() noexcept;

  bool isInCallback() const noexcept;

  static string ms_pubUrl;         // Address of the data sending service
  static const char* ms_startTag;  // Start marker of a batch data receiving
  static const char* ms_endTag;    // End marker of a batch data receiving
  static const char*
      ms_spotTopic;  // Topic subscribed from the data sending service
  static const size_t
      ms_startTagLength;  // Length of the batch receiving start marker
  static const size_t
      ms_endTagLength;  // Length of the batch receiving end marker
  static const size_t
      ms_spotTopicLength;  // Length of the subscribed topic marker

 private:
  SpotAgent(const SpotAgent&) = delete;
  SpotAgent(SpotAgent&&) = delete;
  SpotAgent& operator=(const SpotAgent&) = delete;
  SpotAgent& operator=(SpotAgent&&) = delete;

  unique_ptr<SpotRecord> parseFlatSpot(const hayaku::flat::Spot* spot);
  void parseSpotData(const void* buf, size_t buf_len,
                     const Datetime& startReceiveTime);

  void work_thread();

 private:
  enum STATUS {
    WAITING,
    RECEIVING
  };  // Waiting for a new batch of data, or receiving a batch
      // of data
  enum STATUS status_ = WAITING;  // Current internal state
  std::mutex run_mutex_;          // Prevents multi-threaded start / stop
  std::atomic_bool stop_ = true;  // Flag for ending the agent work
  std::atomic_bool connected_ =
      false;  // Whether the data service has been connected
  std::atomic_bool cleanup_pending_ =
      false;  // Worker resources still require an external join

  int rev_timeout_ = 100;       // Timeout for connecting the data service (ms)
  std::thread receive_thread_;  // Data receiving thread
  std::unique_ptr<ThreadPool>
      tg_;               // Thread pool for the data processing tasks
  size_t work_num_ = 1;  // Number of the threads in the data processing
                          // task thread pool
  std::unique_ptr<ThreadPool> receive_data_tg_;  // Data receiving task group

  bool print_ = true;   // Whether to print the connection information
  string server_addr_;  // Server address

  // The following attributes need to be locked when they are modified, so that
  // strategy can be run in a multi-threaded way
  std::mutex mutex_;
  list<std::function<void(const SpotRecord&)>>
      process_list_;  // List of the registered spot
                      // handlers
  list<std::function<void(Datetime)>>
      post_process_list_;  // List of the registered batch
                          // post-processing functions
};

}  // namespace hayaku
