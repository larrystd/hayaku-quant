#!/usr/bin/python
# -*- coding: utf8 -*-

"""Stable Python API smoke tests used by the interface-boundary refactoring."""

import unittest

import hikyuu
from hikyuu._public_api import TOP_LEVEL_PUBLIC_API


class PublicApiTest(unittest.TestCase):

    def test_stable_top_level_symbols(self):
        self.assertEqual(set(hikyuu.__all__), TOP_LEVEL_PUBLIC_API)
        self.assertLessEqual(len(hikyuu.__all__), 30)
        self.assertTrue(all(hasattr(hikyuu, name) for name in hikyuu.__all__))

    def test_stable_symbols_are_importable(self):
        namespace = {}
        exec("from hikyuu import Stock, KData, Query, Indicator, DataEngine, ExecutionEngine, "
             "StrategyEngine", namespace)
        self.assertIs(namespace["Stock"], hikyuu.Stock)
        self.assertIs(namespace["ExecutionEngine"], hikyuu.ExecutionEngine)
        self.assertIs(namespace["StrategyEngine"], hikyuu.StrategyEngine)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(PublicApiTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
