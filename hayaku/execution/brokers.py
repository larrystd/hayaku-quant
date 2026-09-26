"""Small Python broker callbacks for execution integrations."""

import json

from hayaku.core import OrderBrokerBase
from hayaku.util import hayaku_error

from .broker_easytrader import EasyTraderOrderBroker
from .broker_mail import MailOrderBroker


class OrderBrokerAdapter(OrderBrokerBase):
    """Adapt a small Python ``buy``/``sell`` callback object to the native broker protocol."""

    def __init__(self, broker, name="PYTHON_BROKER"):
        super().__init__(name)
        self._broker = broker

    def _buy(self, datetime, market, code, price, num, stoploss, goal_price, origin, remark=""):
        self._broker.buy(market, code, price, num, stoploss, goal_price, origin, remark)

    def _sell(self, datetime, market, code, price, num, stoploss, goal_price, origin, remark=""):
        self._broker.sell(market, code, price, num, stoploss, goal_price, origin, remark)

    def _get_asset_info(self):
        try:
            if not hasattr(self._broker, "get_asset_info"):
                return ""
            value = self._broker.get_asset_info()
            return json.dumps(value) if isinstance(value, dict) else str(value)
        except Exception as error:
            hayaku_error(str(error))
            return ""


class TestOrderBroker:
    """Broker callback useful in examples; it only prints planned orders."""

    def buy(self, market, code, price, num, stoploss, goal_price, origin, remark=""):
        print(
            f"Buy: {market}{code}, price: {price}, number: {num}, "
            f"expected stop-loss price: {stoploss}, expected goal price: {goal_price}, "
            f"order origin: {origin}, remark: {remark}"
        )

    def sell(self, market, code, price, num, stoploss, goal_price, origin, remark=""):
        print(
            f"Sell: {market}{code}, price: {price}, number: {num}, "
            f"order origin: {origin}, remark: {remark}"
        )


__all__ = (
    "EasyTraderOrderBroker",
    "MailOrderBroker",
    "OrderBrokerAdapter",
    "TestOrderBroker",
)
