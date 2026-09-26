#pragma once

/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-04-25
 *      Author: fasiondog
 */

#include "common/Config.h"
#if !HAYAKU_ENABLE_NODE
#error "Don't enable node client, please config with --node=y"
#endif

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>

#include <atomic>

#include "NodeMessage.h"
#include "common/time/Datetime.h"

namespace hayaku {

class NodeClient {
 public:
  NodeClient() = default;

  explicit NodeClient(const std::string& serverAddr)
      : server_addr_(serverAddr) {}

  virtual ~NodeClient() { close(); }

  /** Set the server address */
  void setServerAddr(const std::string& serverAddr) {
    server_addr_ = serverAddr;
  }

  /** Connect to the server */
  bool dial() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    close();
    // HAYAKU_TRACE("dial: {}", m_server_addr);
    int rv = nng_req0_open(&socket_);
    // HAYAKU_ERROR_IF_RETURN(rv != 0, false, "Failed open req socket! {}",
    // nng_strerror(rv));
    HAYAKU_IF_RETURN(rv != 0, false);
    connected_ = true;

    try {
      // Set the socket connection parameters for sending the result
      rv = nng_socket_set_ms(socket_, NNG_OPT_RECONNMINT, 10);
      NODE_NNG_CHECK(rv, "Failed nng_socket_set_ms!");

      rv = nng_socket_set_ms(socket_, NNG_OPT_RECONNMAXT, 15000);
      NODE_NNG_CHECK(rv, "Failed nng_socket_set_ms!");

      rv = nng_socket_set_ms(socket_, NNG_OPT_SENDTIMEO, 10000);
      NODE_NNG_CHECK(rv, "Failed nng_socket_set_ms!");

      rv = nng_socket_set_ms(socket_, NNG_OPT_RECVTIMEO, 10000);
      NODE_NNG_CHECK(rv, "Failed nng_socket_set_ms!");

      rv = nng_dial(socket_, server_addr_.c_str(), NULL, 0);
      NODE_NNG_CHECK(rv, "Failed dial server: {}!", server_addr_);

      return true;

    } catch (const std::exception& e) {
      HAYAKU_ERROR_IF(show_log_, "Failed dail server: {}! {}", server_addr_,
                      e.what());
    } catch (...) {
      HAYAKU_ERROR_IF(show_log_, "Failed dail server: {}! Unknown error!",
                      server_addr_);
    }

    connected_ = false;
    nng_close(socket_);
    return false;
  }

  /** Close the connection */
  void close() noexcept {
    if (connected_) {
      nng_close(socket_);
      connected_ = false;
    }
  }

  /** Current connection state */
  bool connected() const { return connected_; }

  /** Get the time of the last received server response */
  Datetime getLastAckTime() const { return last_ack_time_; }

  /**
   * Send a message
   * @param req the request message to be sent
   * @param res the returned response
   */
  bool post(const json& req, json& res) noexcept {
    // Guarantee that the communication with the server must be in the req/res
    // mode
    std::lock_guard<std::mutex> lock(mutex_);
    return _send(req) && _recv(res);
  }

  void showLog(bool show) { show_log_ = show; }

 private:
  bool _send(const json& req) const noexcept {
    bool success = false;
    // HAYAKU_ERROR_IF_RETURN(!m_connected, success, "Not connected!");
    HAYAKU_IF_RETURN(!connected_, success);

    nng_msg* msg = nullptr;
    int rv = nng_msg_alloc(&msg, 0);
    // HAYAKU_ERROR_IF_RETURN(rv != 0, success, "Failed nng_msg_alloc! {}",
    // nng_strerror(rv));
    HAYAKU_IF_RETURN(rv != 0, success);

    try {
      encodeMsg(msg, req);
      rv = nng_sendmsg(socket_, msg, 0);
      NODE_NNG_CHECK(rv, "Failed nng_sendmsg!");
      success = true;

    } catch (const std::exception& e) {
      HAYAKU_ERROR_IF(show_log_, "Failed send result! {}", e.what());
    } catch (...) {
      HAYAKU_ERROR_IF(show_log_, "Failed send result! Unknown error!");
    }

    if (!success) {
      nng_msg_free(msg);
    }

    return success;
  }

  bool _recv(json& res) noexcept {
    bool success = false;
    nng_msg* msg{nullptr};
    int rv = nng_recvmsg(socket_, &msg, 0);
    if (rv != 0) {
      HAYAKU_ERROR_IF(show_log_, "Failed nng_recvmsg! {}", nng_strerror(rv));
      return success;
    }

    last_ack_time_ = Datetime::now();

    try {
      res = decodeMsg(msg);
      success = true;

    } catch (const std::exception& e) {
      HAYAKU_ERROR_IF(show_log_, "Failed recv response! {}", e.what());
    } catch (...) {
      HAYAKU_ERROR_IF(show_log_, "Failed recv response! Unknown error!");
    }

    nng_msg_free(msg);
    return success;
  }

 private:
  std::mutex mutex_;
  std::string server_addr_;  // Server address
  nng_socket socket_;
  Datetime last_ack_time_{
      Datetime::now()};  // The time of the last received server response
  std::atomic_bool connected_{false};
  std::atomic_bool show_log_{true};
};

}  // namespace hayaku
