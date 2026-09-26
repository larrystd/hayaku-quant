#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#include "application/plugins/PluginBase.h"
#include "data/RealtimeDataSource.h"

namespace hayaku {

/**
 * The shm data service plugin interface
 * @ingroup DataDriver
 */
class ShmServerPluginInterface : public PluginBase, public RealtimeDataSource {
 public:
  static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;
  ShmServerPluginInterface() = default;
  virtual ~ShmServerPluginInterface() = default;

  /**
   * Start the shm server
   * @param datadir data directory; the current data directory of the data
   * runtime is used when it is empty
   * @param publish_shm whether to publish the two kinds of shared memory
   * snapshots
   * @param recv_spot whether this process receives the realtime market data (it
   * calls startSpotAgent internally)
   * @return true is returned on a successful start; it refuses and returns
   * false when this process is already in the client mode (to prevent
   * self-connection, see design §5.5)
   */
  virtual bool start(const std::string& datadir, bool publish_shm,
                     bool recv_spot) noexcept = 0;

  /** Stop the shm server, unregister the mirror hook and the loading event
   * callbacks, and release the shared memory segment */
  virtual void stop() noexcept = 0;

  /** Whether the service is running */
  virtual bool running() const noexcept = 0;

  /** The service listening address (for logs / troubleshooting; an empty string
   * when it is not started) */
  virtual const std::string& addr() const noexcept = 0;
};

}  // namespace hayaku
