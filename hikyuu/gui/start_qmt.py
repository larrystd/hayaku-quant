#!/usr/bin/python3
# -*- coding: utf-8 -*-


from hikyuu.fetcher.stock.zh_stock_a_qmt import parse_one_result_qmt, get_spot
from hikyuu.gui.spot_server import release_nng_senders, start_send_spot, end_send_spot, send_spot
from hikyuu.util import hku_info


def callback(datas):
    records = []
    for stock_code, data in datas.items():
        records.append(parse_one_result_qmt(stock_code, data))
    hku_info(f"接收: {len(records)}")

    if records:
        start_send_spot()
        send_spot(records)
        end_send_spot()


if __name__ == "__main__":
    import os

    from hikyuu.data.hku_config_template import generate_default_config
    from hikyuu import Datetime, StrategyContext, TimeDelta, open_session
    from hikyuu.core import Days, constant
    from hikyuu.util import hku_error, hku_info_if

    config_file = os.path.expanduser('~') + "/.hikyuu/hikyuu.ini"
    if not os.path.exists(config_file):
        # 创建默认配置
        hku_info("创建默认配置文件")
        generate_default_config()

    context = StrategyContext(["all"])
    context.ktype_list = ["day"]
    session = open_session(config_file, ignore_preload=True, context=context)

    # 后续希望每次先主动获取一次全部的tick, 这里需要等待所有数据加载完毕，以便保证全部证券收到第一次tick通知
    hku_info("waiting all data loaded ...")
    session.wait_ready()
    data = session.data

    stk_list = [s for s in data if s.valid and s.type in (
        constant.STOCKTYPE_A, constant.STOCKTYPE_INDEX, constant.STOCKTYPE_ETF,
        constant.STOCKTYPE_GEM, constant.STOCKTYPE_START, constant.STOCKTYPE_A_BJ)]

    hku_info("start xtquant")
    code_list = [f'{s.code}.{s.market}' for s in stk_list]
    from xtquant import xtdata
    import time
    # xtdata.subscribe_whole_quote(['SH', 'SZ', 'BJ'], callback)
    batch_size = 250
    n = len(code_list) // batch_size
    for i in range(n):
        codes = code_list[i * batch_size: (i + 1) * batch_size]
        v = xtdata.subscribe_whole_quote(codes, callback)
        hku_info_if(v < 0, "订阅失败")
        time.sleep(0.5)
    codes = code_list[n * batch_size:]
    if codes:
        v = xtdata.subscribe_whole_quote(codes, callback)
        hku_info_if(v < 0, "订阅失败")

    # 每日 9:30 时，主动读取行情一次，以便 hikyuu 生成当日首个分钟线
    while True:
        try:
            today = Datetime.today()
            if today.day_of_week() not in (0, 6) and not data.is_holiday(today):
                hku_info("get full tick ...")
                start_send_spot()
                records = get_spot(stk_list, None, None, send_spot)
                end_send_spot()
            now = Datetime.now()
            today_open = today + TimeDelta(0, 9, 30)
            if now < today_open:
                delta = today_open - Datetime.now()
            else:
                delta = today_open + Days(1) - Datetime.now()
            hku_info(f"start timer: {delta}s")
            time.sleep(delta.total_seconds())
        except KeyboardInterrupt:
            print("Ctrl-C 终止")
            break
        except Exception as e:
            hku_error(str(e))
            time.sleep(10)

    release_nng_senders()

    # try:
    #     xtdata.run()
    # except Exception as e:
    #     hku_error(str(e))
    # finally:
    #     # 退出释放资源
    #     release_nng_senders()
    #     exit(0)
