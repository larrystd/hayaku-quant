#!/usr/bin/python
# -*- coding: utf8 -*-

"""Stable Python API smoke tests used by the interface-boundary refactoring."""

import unittest

import hikyuu


# This is deliberately a small stable subset, not a snapshot of every name currently leaked by
# import *. Adding a symbol here is an API compatibility decision.
STABLE_TOP_LEVEL_API = {
    "Datetime",
    "Indicator",
    "KData",
    "Query",
    "Stock",
    "StockManager",
    "Strategy",
    "System",
    "crtTM",
    "hikyuu_init",
}

STABLE_FACTORY_API = {
    "MA",
    "MM_FixedCount",
    "SG_Cross",
    "ST_FixedPercent",
    "SYS_Simple",
    "TC_Zero",
}


class PublicApiTest(unittest.TestCase):

    def test_stable_top_level_symbols(self):
        missing = sorted(name for name in STABLE_TOP_LEVEL_API if not hasattr(hikyuu, name))
        self.assertEqual(missing, [], f"Missing stable top-level API: {missing}")

    def test_stable_factory_symbols(self):
        missing = sorted(name for name in STABLE_FACTORY_API if not hasattr(hikyuu, name))
        self.assertEqual(missing, [], f"Missing stable factory API: {missing}")

    def test_stable_symbols_are_importable(self):
        namespace = {}
        exec("from hikyuu import Stock, KData, Query, Indicator, Strategy, System", namespace)
        self.assertEqual(namespace["Stock"], hikyuu.Stock)
        self.assertEqual(namespace["System"], hikyuu.System)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(PublicApiTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
