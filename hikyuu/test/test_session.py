#!/usr/bin/python
# -*- coding: utf8 -*-

"""DataEngine and HikyuuSession boundary tests."""

import unittest

import hikyuu
from hikyuu.execution import AccountConfig, AccountId
from test_init import config_file


class SessionTest(unittest.TestCase):

    def test_open_close(self):
        session = hikyuu.open_session(config_file)
        self.assertTrue(session.opened)
        session.wait_ready()
        self.assertTrue(session.ready)
        self.assertGreater(session.data.size, 0)

        session.close()
        self.assertFalse(session.opened)
        session.close()
        with self.assertRaises(hikyuu.HKUException):
            _ = session.data

    def test_context_manager_and_queries(self):
        with hikyuu.open_session(config_file) as session:
            stock = session.data.get_stock("sh000001")
            self.assertEqual(stock.market_code, "SH000001")
            actual_market = session.data.get_market_info("SH")
            self.assertEqual(actual_market.market, "SH")
            self.assertEqual(actual_market.code, "000001")
            self.assertTrue(session.data.get_history_finance_all_fields())
            self.assertEqual(session.data.get_kdata("sh000001", hikyuu.Query(-10)),
                             stock.get_kdata(hikyuu.Query(-10)))

        self.assertFalse(session.opened)

    def test_native_execution_lifecycle(self):
        account_id = AccountId(42)
        account = AccountConfig(name="session-engine", initial_cash=100000.0,
                                account_id=account_id)
        session = hikyuu.open_session(config_file, account_config=account)
        self.assertTrue(session.has_execution)
        self.assertFalse(session.has_strategy)

        execution = session.execution
        self.assertEqual(execution.account_id, account_id)
        self.assertEqual(execution.view().account_id, account_id)
        # Account initialization records the opening CHECKIN transaction.
        self.assertEqual(len(execution.history()), 1)
        session.close()

        self.assertFalse(session.has_execution)
        with self.assertRaises(hikyuu.HKUException):
            execution.snapshot()


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SessionTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
