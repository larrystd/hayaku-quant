#!/usr/bin/python
# -*- coding: utf8 -*-

"""Tests for the declarative Python public-API boundary."""

import importlib.util
from importlib import resources
import sys
import subprocess
import unittest
from pathlib import Path

import hayaku
from hayaku.extensions import spi
import hayaku.extensions.realtime as realtime
from hayaku._public_api import (ADVANCED_API, EXTENSION_SPI_API, INTERNAL_TOP_LEVEL_API,
                                TOP_LEVEL_PUBLIC_API, resolve_public_api)
from hayaku.common import Datetime, HAYAKUException, Parameter, TimeDelta
from hayaku.data import DataEngine
from hayaku.execution import (AccountConfig, AccountId, AccountSnapshot, AccountView,
                              EasyTraderOrderBroker, ExecutionEngine, ExecutionReport,
                              ExecutionStatus, FundsRecord, MailOrderBroker, OrderRequest,
                              OrderBrokerAdapter, OrderOrigin, OrderSide, PositionExtInfo, PositionRecord,
                              TestOrderBroker, TradeRecord,
                              get_order_origin_enum, get_order_origin_name,
                              positions_to_dataframe, positions_to_numpy, trades_to_dataframe,
                              trades_to_numpy)
from hayaku.strategy import (BacktestRequest, BacktestResult, ConditionBase, EnvironmentBase,
                             MoneyManagerBase, MultiFactorBase, NormalizeBase, ProfitGoalBase,
                             ScoresFilterBase, SignalBase, SlippageBase, StoplossBase,
                             StrategyDefinition, StrategyEngine, crtCN,
                             crtEV, crtMF, crtMM, crtNorm, crtPG, crtSCFilter, crtSG,
                             crtSP, crtST)


class ApiBoundaryTest(unittest.TestCase):

    def test_resolved_public_api_is_explicit_and_complete(self):
        resolved = resolve_public_api(vars(hayaku))
        self.assertEqual(len(resolved), len(set(resolved)))
        self.assertLessEqual(len(resolved), 30)
        self.assertEqual(set(resolved), TOP_LEVEL_PUBLIC_API)
        self.assertTrue(set(resolved).isdisjoint(INTERNAL_TOP_LEVEL_API))

    def test_legacy_managers_are_absent(self):
        for name in ("StockManager", "TradeManager", "System", "hayaku_init", "sm"):
            self.assertNotIn(name, hayaku.__all__)
            self.assertFalse(hasattr(hayaku, name))
        self.assertFalse(hasattr(hayaku.core, "StockManager"))
        self.assertFalse(hasattr(hayaku.core, "hayaku_init"))
        self.assertFalse(hasattr(hayaku.core, "TradeManager"))
        self.assertFalse(hasattr(hayaku.core, "System"))

    def test_data_engine_has_domain_import(self):
        self.assertIs(DataEngine, hayaku.DataEngine)

    def test_common_module_has_only_shared_value_types(self):
        common = __import__("hayaku.common", fromlist=["__all__"])
        self.assertEqual(set(common.__all__), {"Datetime", "HAYAKUException", "Parameter", "TimeDelta"})
        self.assertTrue(all((Datetime, HAYAKUException, Parameter, TimeDelta)))

    def test_execution_engine_has_narrow_domain_import(self):
        self.assertIs(ExecutionEngine, hayaku.ExecutionEngine)
        execution_api = {
            "AccountConfig", "AccountId", "AccountSnapshot", "AccountView", "ExecutionEngine",
            "ExecutionReport", "ExecutionStatus", "FundsRecord", "EasyTraderOrderBroker",
            "MailOrderBroker", "OrderBrokerAdapter", "OrderRequest", "OrderSide",
            "PositionExtInfo", "PositionRecord", "TestOrderBroker",
            "OrderOrigin", "TradeRecord", "get_order_origin_enum", "get_order_origin_name",
            "positions_to_dataframe", "positions_to_numpy",
            "trades_to_dataframe", "trades_to_numpy"
        }
        self.assertEqual(set(__import__("hayaku.execution", fromlist=["__all__"]).__all__),
                         execution_api)
        self.assertTrue(all((AccountConfig, AccountId, AccountSnapshot, AccountView,
                             EasyTraderOrderBroker, ExecutionReport, ExecutionStatus, FundsRecord,
                             MailOrderBroker, OrderBrokerAdapter, OrderRequest, OrderSide,
                             PositionExtInfo, PositionRecord,
                             OrderOrigin, TestOrderBroker, TradeRecord, get_order_origin_enum,
                             get_order_origin_name, positions_to_dataframe,
                             positions_to_numpy, trades_to_dataframe, trades_to_numpy)))

    def test_strategy_engine_has_narrow_domain_import(self):
        self.assertIs(StrategyEngine, hayaku.StrategyEngine)
        strategy_api = {
            "BacktestResult", "ConditionBase", "EnvironmentBase",
            "MoneyManagerBase", "MultiFactorBase", "NormalizeBase", "ProfitGoalBase",
            "ScoresFilterBase", "SignalBase", "SlippageBase", "StoplossBase",
            "BacktestRequest", "StrategyDefinition", "StrategyEngine", "crtCN",
            "crtEV", "crtMF", "crtMM", "crtNorm", "crtPG", "crtSCFilter", "crtSG",
            "crtSP", "crtST"
        }
        self.assertEqual(set(__import__("hayaku.strategy", fromlist=["__all__"]).__all__),
                         strategy_api)
        self.assertTrue(all((BacktestRequest, BacktestResult, ConditionBase, EnvironmentBase,
                             MoneyManagerBase, MultiFactorBase, NormalizeBase, ProfitGoalBase,
                             ScoresFilterBase, SignalBase, SlippageBase, StoplossBase,
                             StrategyDefinition, crtCN, crtEV, crtMF, crtMM,
                             crtNorm, crtPG, crtSCFilter, crtSG, crtSP, crtST)))

    def test_legacy_python_packages_are_removed(self):
        self.assertIsNone(importlib.util.find_spec("hayaku.trade_manage"))
        self.assertIsNone(importlib.util.find_spec("hayaku.trade_sys"))
        for module in ("hayaku.fetcher", "hayaku.util", "hayaku.flat", "hayaku.extend",
                       "hayaku.advanced", "hayaku.draw", "hayaku.gui", "hayaku.shell",
                       "hayaku.interactive", "hayaku.hub", "hayaku.config",
                       "hayaku.test", "hayaku.examples", "hayaku.indicator",
                       "hayaku.analysis", "hayaku.apps", "hayaku.ingest",
                       "hayaku.realtime", "hayaku.visualization", "hayaku.spi",
                       "hayaku.session"):
            self.assertIsNone(importlib.util.find_spec(module), module)
        package_root = Path(hayaku.__file__).resolve().parent
        self.assertFalse((package_root / "gui/data").exists())
        self.assertFalse((package_root / "draw/drawplot").exists())
        self.assertFalse((package_root / "test").exists())
        self.assertFalse((package_root / "examples").exists())

    def test_moved_package_resources_are_locatable(self):
        package_files = resources.files("hayaku")
        for resource in (
            "extensions/ingest/schema/sqlite/createdb.sql",
            "extensions/ingest/schema/mysql/createdb.sql",
            "extensions/ingest/schema/clickhouse/createdb.sql",
            "extensions/ingest/schema/memory/createdb.sql",
            "application/gui/resources/MainWindow.ui",
            "application/gui/resources/images/hayaku_small.png",
            "extensions/visualization/resources/icons/1.png",
        ):
            self.assertTrue(package_files.joinpath(resource).is_file(), resource)

    def test_execution_record_conversion_uses_domain_origin(self):
        self.assertEqual(get_order_origin_name(OrderOrigin.SIGNAL), "SG")
        self.assertEqual(get_order_origin_enum("SG"), OrderOrigin.SIGNAL)
        records = trades_to_numpy([TradeRecord()])
        self.assertIn("origin", records.dtype.names)
        self.assertNotIn("part_from", records.dtype.names)

    def test_execution_account_accepts_explicit_broker_adapter(self):
        broker = OrderBrokerAdapter(TestOrderBroker(), name="test")
        config = AccountConfig(brokers=[broker])
        self.assertEqual(len(config.brokers), 1)
        self.assertEqual(config.brokers[0].name, "test")

    def test_package_dir_uses_reviewed_public_boundary(self):
        self.assertEqual(set(dir(hayaku)), set(hayaku.__all__))

    def test_package_import_has_no_optional_or_runtime_side_effect(self):
        code = r'''
import sys

before = set(sys.modules)
import hayaku
loaded = set(sys.modules) - before
banned = ("hayaku.application.hub", "hayaku.extensions.visualization",
          "hayaku.application.gui", "hayaku.extensions.ingest",
          "hayaku.extensions.realtime", "hayaku_ingest_native", "hayaku_realtime_native",
          "matplotlib", "pandas", "sqlalchemy", "PySide6", "PyQt5", "PyQt6")
unexpected = sorted(
    name for name in loaded
    if any(name == prefix or name.startswith(prefix + ".") for prefix in banned)
)
assert not unexpected, unexpected
assert not hasattr(hayaku, "hayaku_cleanup")
'''
        project_root = Path(__file__).resolve().parents[2]
        result = subprocess.run(
            [sys.executable, "-c", code], cwd=project_root, capture_output=True, text=True,
            check=False)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_extension_spi_is_explicit_only(self):
        self.assertEqual(set(spi.__all__), EXTENSION_SPI_API)
        for name in spi.__all__:
            self.assertFalse(hasattr(hayaku, name))

    def test_realtime_api_is_explicit_only(self):
        self.assertEqual(set(realtime.__all__), ADVANCED_API)
        for name in realtime.__all__:
            self.assertFalse(hasattr(hayaku, name))


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ApiBoundaryTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
