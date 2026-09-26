#!/usr/bin/python
# -*- coding: utf8 -*-

"""Stable Python API smoke tests used by the interface-boundary refactoring."""

import unittest

import hayaku
from hayaku._public_api import TOP_LEVEL_PUBLIC_API


class PublicApiTest(unittest.TestCase):

    def test_stable_top_level_symbols(self):
        self.assertEqual(set(hayaku.__all__), TOP_LEVEL_PUBLIC_API)
        self.assertLessEqual(len(hayaku.__all__), 30)
        self.assertTrue(all(hasattr(hayaku, name) for name in hayaku.__all__))

    def test_stable_symbols_are_importable(self):
        namespace = {}
        exec("from hayaku import Stock, KData, Query, Indicator, DataEngine, ExecutionEngine, "
             "StrategyEngine", namespace)
        self.assertIs(namespace["Stock"], hayaku.Stock)
        self.assertIs(namespace["ExecutionEngine"], hayaku.ExecutionEngine)
        self.assertIs(namespace["StrategyEngine"], hayaku.StrategyEngine)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(PublicApiTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
