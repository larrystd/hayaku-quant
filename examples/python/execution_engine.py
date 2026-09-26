"""Submit one in-memory order through the explicit execution engine."""

from hayaku import Query, open_session
from hayaku.execution import AccountConfig, OrderRequest, OrderSide


def main():
    account = AccountConfig(name="example", initial_cash=100_000.0)
    with open_session(account_config=account) as session:
        stock = session.data.get_stock("sh600000")
        if stock.is_null():
            raise RuntimeError("example stock sh600000 is unavailable")
        kdata = session.data.get_kdata("sh600000", Query(-1))
        if not kdata:
            raise RuntimeError("example stock sh600000 has no price data")
        last = kdata[-1]
        request = OrderRequest(
            OrderSide.BUY,
            last.datetime,
            stock,
            real_price=last.close,
            number=100,
        )
        report = session.execution.submit(request)
        print(report.trade)


if __name__ == "__main__":
    main()
