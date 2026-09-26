/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <condition_variable>
#include <mutex>

#include "RealtimePort.h"
#include "common/Log.h"

namespace hayaku {

namespace {

struct PortState {
    std::mutex mutex;
    std::condition_variable changed;
    const RealtimePort* port{nullptr};
    size_t activeCalls{0};
    bool unregistering{false};
};

PortState& portState() {
    // Live modules may register before the process runtime is initialized.
    static auto* state = new PortState;
    return *state;
}

thread_local size_t g_portCallDepth = 0;
thread_local size_t g_realtimeCallbackDepth = 0;

void finishPortCall() noexcept {
    auto& state = portState();
    --g_portCallDepth;
    std::lock_guard<std::mutex> lock(state.mutex);
    --state.activeCalls;
    if (state.activeCalls == 0) {
        state.changed.notify_all();
    }
}

template <typename Function>
bool withRealtimePort(Function&& func) {
    auto& state = portState();
    const RealtimePort* port;
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        if (!state.port || state.unregistering) {
            return false;
        }
        port = state.port;
        ++state.activeCalls;
        ++g_portCallDepth;
    }

    try {
        func(*port);
    } catch (...) {
        finishPortCall();
        throw;
    }
    finishPortCall();
    return true;
}

}  // namespace

bool registerRealtimePort(const RealtimePort* port) noexcept {
    if (!port || !port->stopSpot || !port->startStrategySpot || !port->reloadAround ||
        !port->shutdown || !port->quiescent) {
        return false;
    }
    auto& state = portState();
    std::lock_guard<std::mutex> lock(state.mutex);
    if (state.unregistering || (state.port && state.port != port)) {
        return false;
    }
    state.port = port;
    return true;
}

bool unregisterRealtimePort(const RealtimePort* port) noexcept {
    if (!port || g_portCallDepth != 0 || detail::spotAgentCallbackActive()) {
        return false;
    }
    auto& state = portState();
    std::unique_lock<std::mutex> lock(state.mutex);
    if (state.unregistering || state.port != port) {
        return false;
    }
    state.unregistering = true;
    state.changed.wait(lock, [&] { return state.activeCalls == 0; });
    bool quiet = port->quiescent();
    if (quiet) {
        state.port = nullptr;
    }
    state.unregistering = false;
    lock.unlock();
    state.changed.notify_all();
    return quiet;
}

void stopRealtimeForStrategy() {
    withRealtimePort([](const RealtimePort& port) { port.stopSpot(); });
}

void startRealtimeForStrategy(const RealtimeSpotProcess& process,
                              const RealtimePostProcess& postProcess, size_t workerNum,
                              const string& address) {
    HAYAKU_CHECK(withRealtimePort([&](const RealtimePort& port) {
                  port.startStrategySpot(process, postProcess, workerNum, address);
              }),
              "Realtime extension is not installed; Strategy::start(true) requires live support");
}

void reloadWithRealtimePaused(void (*reload)()) {
    HAYAKU_CHECK(reload, "A reload callback is required");
    if (!withRealtimePort([&](const RealtimePort& port) { port.reloadAround(reload); })) {
        reload();
    }
}

void shutdownRealtimeRuntime() noexcept {
    try {
        withRealtimePort([](const RealtimePort& port) { port.shutdown(); });
    } catch (const std::exception& e) {
        HAYAKU_ERROR("Failed to shut down realtime runtime: {}", e.what());
    } catch (...) {
        HAYAKU_ERROR("Failed to shut down realtime runtime");
    }
}

void detail::enterRealtimeCallback() noexcept {
    ++g_realtimeCallbackDepth;
}

void detail::leaveRealtimeCallback() noexcept {
    --g_realtimeCallbackDepth;
}

bool detail::spotAgentCallbackActive() noexcept {
    return g_realtimeCallbackDepth != 0;
}

}  // namespace hayaku
