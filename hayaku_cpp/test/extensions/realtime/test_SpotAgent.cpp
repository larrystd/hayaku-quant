/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>

#include "doctest/doctest.h"
#include <application/HayakuSession.h>
#include <application/SessionOptions.h>
#include <extensions/realtime/SpotAgent.h>
#include <extensions/realtime/spot_generated.h>
#include <data/DataRuntime.h>

using namespace hayaku;

namespace {

struct PublisherGuard {
    nng_socket socket{};
    bool opened{false};

    ~PublisherGuard() {
        if (opened) {
            nng_close(socket);
        }
    }
};

struct QuotationServerReset {
    ~QuotationServerReset() {
        SpotAgent::setQuotationServer("ipc:///tmp/hayaku_real.ipc");
    }
};

}  // namespace

/**
 * @defgroup test_spotagent_suite test_spotagent_suite
 * @ingroup test_hayaku_application_suite
 * @{
 */

TEST_CASE("test_SpotAgent_stop_from_callback") {
    PublisherGuard publisher;
    QuotationServerReset resetServer;
    REQUIRE_EQ(nng_pub0_open(&publisher.socket), 0);
    publisher.opened = true;
    std::string address;
    nng_listener listener{};
#if HAYAKU_OS_WINDOWS
    int listenResult = NNG_EADDRINUSE;
    const auto seed = static_cast<unsigned>(
      std::chrono::steady_clock::now().time_since_epoch().count() % 20000);
    for (unsigned attempt = 0; attempt < 128 && listenResult == NNG_EADDRINUSE; ++attempt) {
        const auto port = 30000 + (seed + attempt) % 20000;
        address = "tcp://127.0.0.1:" + std::to_string(port);
        listenResult = nng_listen(publisher.socket, address.c_str(), &listener, 0);
    }
#else
    const auto unique = static_cast<unsigned long long>(
      std::chrono::steady_clock::now().time_since_epoch().count());
    address = "ipc:///tmp/hayaku-spotagent-stop-" + std::to_string(unique) + ".ipc";
    const int listenResult = nng_listen(publisher.socket, address.c_str(), &listener, 0);
#endif
    if (listenResult == NNG_EPERM) {
        WARN_MESSAGE(false, "NNG listener is not permitted in this sandbox; callback test skipped");
        return;
    }
    REQUIRE_EQ(listenResult, 0);

    const auto& runtime = getDataRuntime();
    SessionOptions options(runtime.getBaseInfoDriverParameter(), runtime.getBlockDriverParameter(),
                           runtime.getKDataDriverParameter(), runtime.getPreloadParameter(),
                           runtime.getHayakuParameter(), runtime.getStrategyContext());
    auto session = HayakuSession::open(options);
    SpotAgent agent;
    SpotAgent::setQuotationServer(address);
    std::atomic_bool callbackEntered{false};
    std::atomic_bool callbackReturned{false};
    std::atomic_bool sessionCloseRejected{false};
    agent.addProcess([&](const SpotRecord&) {
        callbackEntered = true;
        agent.stop();
        session.close();
        sessionCloseRejected = session.isOpen();
        callbackReturned = true;
    });
    agent.start();

    /** @arg A connected agent receives a flatbuffer batch from the NNG publisher. */
    for (int i = 0; i < 200 && !agent.isConnected(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    CHECK_UNARY(agent.isConnected());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    flatbuffers::FlatBufferBuilder builder;
    auto spot = hayaku::flat::CreateSpotDirect(builder, "SH", "000001", "test",
                                               "2024-01-02 09:31:00");
    std::vector<flatbuffers::Offset<hayaku::flat::Spot>> spots{spot};
    hayaku::flat::FinishSpotListBuffer(builder, hayaku::flat::CreateSpotListDirect(builder, &spots));
    std::vector<uint8_t> packet{':', 's', 'p', 'o', 't', ':'};
    packet.insert(packet.end(), builder.GetBufferPointer(),
                  builder.GetBufferPointer() + builder.GetSize());

    for (int i = 0; i < 20 && !callbackReturned; ++i) {
        const char start[] = ":spot:[start spot]";
        CHECK_EQ(nng_send(publisher.socket, const_cast<char*>(start), sizeof(start) - 1, 0), 0);
        CHECK_EQ(nng_send(publisher.socket, packet.data(), packet.size(), 0), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    /** @arg Stopping from a handler returns without joining its own worker thread. */
    CHECK_UNARY(callbackEntered.load());
    CHECK_UNARY(callbackReturned.load());
    /** @arg Session teardown is refused while the callback owns an agent worker. */
    CHECK_UNARY(sessionCloseRejected.load());
    CHECK_UNARY_FALSE(agent.isRunning());

    /** @arg The owner can subsequently join and release all worker resources. */
    agent.stop();
    CHECK_UNARY_FALSE(agent.isConnected());
    CHECK_NOTHROW(agent.stop());

    /** @arg After the external join, the same agent may start and stop again. */
    CHECK_NOTHROW(agent.start());
    for (int i = 0; i < 200 && !agent.isConnected(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    CHECK_UNARY(agent.isConnected());
    agent.stop();
    session.close();
    CHECK_UNARY_FALSE(session.isOpen());
}

/** @} */
