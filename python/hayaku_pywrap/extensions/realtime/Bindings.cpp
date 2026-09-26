#include "Bindings.h"

/* Domain binding registrations. */

// Registration group: _SpotAgent
/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-30
 *     Author: fasiondog
 */

#include <extensions/realtime/GlobalSpotAgent.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_SpotAgent(py::module& m) {
  m.def("start_spot_agent", startSpotAgent, py::arg("print") = false,
        py::arg("worker_num") = 1, py::arg("addr") = string(),
        R"(start_spot_agent([print=False, worker_num=1, addr=""])

    Start the market data receiving agent

    :param print: whether to print the logs
    :param worker_num: the number of the working threads
    :param addr: the market data collection service address)");

  m.def("stop_spot_agent", stopSpotAgent,
        "Stop the market data receiving agent");
  m.def("spot_agent_is_running", spotAgentIsRunning,
        "Judge whether the market data receiving agent is running");
  m.def("spot_agent_is_connected", spotAgentIsConnected,
        "Judge whether the market data receiving agent is connected");
}

// Registration group: _dataserver
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include <extensions/realtime/DataServerPlugin.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

void export_plugin_dataserver(py::module& m) {
  m.def(
      "start_data_server", startDataServer,
      py::arg("addr") = "tcp://0.0.0.0:9201", py::arg("work_num") = 3,
      py::arg("save_tick") = false, py::arg("buf_tick") = false,
      py::arg("parquet_path") = std::string(),
      R"(start_data_server(addr: str[, work_num: int=3, save_tick: bool=False, buf_tick: bool=False, parquet_path: str=''])

    Start the data cache server. The save_tick parameter is related to parquet_path:
    - If save_tick=True and parquet_path is not empty, use parquet_path to save the data;
    - If save_tick=True and parquet_path is empty, use the clickhouse K-line storage engine to save the data (it needs to be configured to use the clickhouse K-line storage engine)

    :param str addr: the server address
    :param int work_num: the number of the working threads
    :param bool save_tick: whether to save the tick data to the database (when parquet_path is not empty, save with the parquet files; otherwise, the clickhouse K-line storage engine needs to be used)
    :param bool buf_tick: whether to cache the tick data
    :param str parquet_path: the parquet file path to save the tick data, valid only when save_tick=True
    :return: None)");

  m.def("stop_data_server", stopDataServer, R"(stop_data_server()

    Stop the data cache server)");

  m.def(
      "get_data_from_buffer_server", getDataFromBufferServer,
      R"(get_data_from_buffer_server(addr: str, stklist: list, ktype: Query.KType)

    Pull and update the latest cached data from the dataserver data cache server

    :param str addr: the data server address, e.g.: tcp://192.168.1.1:9201
    :param list stklist: the stock list whose data needs to be obtained
    :param Query.KType ktype: the data type)");

  m.def(
      "get_spot_from_buffer_server", getSpotFromBufferServer,
      R"(get_spot_from_buffer_server(addr: str, market: str, code: str, datetime: str)

    Get the cached spot data of the specified security greater than or equal to the specified date from the dataserver

    :param str addr: the data server address, e.g.: tcp://192.168.1.1:9201
    :param str market: the market code
    :param str code: the stock code
    :param str datetime: the queried date)");
}

// Registration group: _shmserver
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#include <extensions/realtime/ShmServerPlugin.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

void export_plugin_shmserver(py::module& m) {
  m.def(
      "start_shm_server", startShmServer, py::arg("datadir") = std::string(),
      py::arg("publish_shm") = true, py::arg("recv_spot") = true,
      R"(start_shm_server(datadir: str='', publish_shm: bool=True, recv_spot: bool=True) -> bool

    Start the shm (shared memory) data server within the current process, for the other hayaku processes to read with zero copy as the clients, avoiding the repeated data loading of the multiple processes.
    The service is provided by the standalone shmserver plugin (a VIP plugin, requiring a valid license).
    The client processes need to explicitly enable use_shm_server (in the configuration file or load_hayaku(use_shm_server=True)) to join this service.

    It must be called after the hayaku initialization (import hayaku completes the initialization by default); calling it before the initialization will return False because the data is not ready.

    :param str datadir: the data directory; when empty, the active data runtime directory is used
    :param bool publish_shm: whether to publish the two kinds of the shared memory snapshots (the K-line hot data + the basic information)
    :param bool recv_spot: whether this process receives the real-time market data (internally subscribing to quotation_server and driving the real-time updates)
    :return: return True when started successfully; return False when this process is already in the client mode, the plugin is missing or the license is invalid)");

  m.def("stop_shm_server", stopShmServer, R"(stop_shm_server() -> None

    Stop the shm data server within the current process, releasing the shared memory segments and unregistering the related hooks)");

  m.def("is_shm_server_running", isShmServerRunning,
        R"(is_shm_server_running() -> bool

    Query whether the shm data server within the current process is running

    :return: return True when running, otherwise False)");
}

void bindRealtime(py::module_& m) {
  export_SpotAgent(m);
  export_plugin_dataserver(m);
  export_plugin_shmserver(m);
}
