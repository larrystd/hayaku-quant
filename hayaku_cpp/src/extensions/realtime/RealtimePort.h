#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Optional realtime integration port. The research/backtest core owns the port;
 * a live extension may register its implementation when it is loaded.
 */

#include <functional>

#include "extensions/realtime/SpotRecord.h"

namespace hayaku {

using RealtimeSpotProcess = std::function<void(const SpotRecord&)>;
using RealtimePostProcess = std::function<void(Datetime)>;

struct RealtimePort {
  void (*stopSpot)();
  void (*startStrategySpot)(const RealtimeSpotProcess&,
                            const RealtimePostProcess&, size_t, const string&);
  void (*reloadAround)(void (*)());
  void (*shutdown)();
  bool (*quiescent)() noexcept;
};

/**
 * Register one live implementation. The port and its code must outlive
 * registration. A dynamic library may be unloaded only after every
 * session/service has stopped and unregisterRealtimePort returns true;
 * concurrent unload is not supported.
 */
bool HAYAKU_API registerRealtimePort(const RealtimePort* port) noexcept;

/** Reject registration removal while a callback, service or port call remains
 * active. */
bool HAYAKU_API unregisterRealtimePort(const RealtimePort* port) noexcept;

/** A no-op without live; only Strategy::start(true) requires an installed port.
 */
void HAYAKU_API stopRealtimeForStrategy();
void HAYAKU_API startRealtimeForStrategy(const RealtimeSpotProcess& process,
                                         const RealtimePostProcess& postProcess,
                                         size_t workerNum,
                                         const string& address);
void HAYAKU_API reloadWithRealtimePaused(void (*reload)());
void HAYAKU_API shutdownRealtimeRuntime() noexcept;

namespace detail {
void HAYAKU_API enterRealtimeCallback() noexcept;
void HAYAKU_API leaveRealtimeCallback() noexcept;
bool HAYAKU_API spotAgentCallbackActive() noexcept;
}  // namespace detail

}  // namespace hayaku
