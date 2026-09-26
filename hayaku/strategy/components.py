# -*- coding: utf8 -*-

"""Python factories for custom strategy components.

These factories are intentionally explicit strategy-domain extensions. They are not imported by
the lightweight :mod:`hayaku` package root.
"""

from hayaku.core import (
    ConditionBase, EnvironmentBase, MoneyManagerBase,
    ProfitGoalBase, SignalBase, SlippageBase, StoplossBase,
    MultiFactorBase, ScoresFilterBase, NormalizeBase
)


def part_iter(self):
    for i in range(len(self)):
        yield self[i]


ConditionBase.__iter__ = part_iter


def part_init(self, name='', params=None):
    super(self.__class__, self).__init__(name)
    self._name = name
    params = {} if params is None else params
    self._params = params
    for k, v in params.items():
        self.set_param(k, v)


def part_clone(self):
    cloned = self.__class__.__new__(self.__class__)
    self.__class__.__init__(cloned, self)
    cloned.__dict__.update(self.__dict__)
    return cloned


# ------------------------------------------------------------------
# condition
# ------------------------------------------------------------------
def crtCN(func, params=None, name='crtCN'):
    """
    Quickly create a system validity condition

    :param func: the system validity condition function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom system validity condition instance
    """
    meta_x = type(name, (ConditionBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    # Force importing the global namespace, to avoid losing the virtual function interfaces of the custom inheritance when used by the hub
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# environment
# ------------------------------------------------------------------
def crtEV(func, params=None, name='crtEV'):
    """
    Quickly create a market environment strategy

    :param func: the market environment strategy function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom market environment strategy instance
    """
    meta_x = type(name, (EnvironmentBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# moneymanager
# ------------------------------------------------------------------
def crtMM(get_buy_num, get_sell_num=None, params=None, name='crtMM', buy_notify=None, sell_notify=None):
    """
    Quickly create a money management strategy

    :param get_buy_num: the interface to get the buy number
    :param get_sell_num: the interface to get the sell number, defaults to None (sell all)
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param buy_notify: receive the notification of the buy trade records
    :param sell_notify: receive the notification of the sell trade records
    :return: the custom money management strategy instance
    """
    meta_x = type(name, (MoneyManagerBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._get_buy_num = get_buy_num
    if get_sell_num is not None:
        meta_x._get_sell_num = get_sell_num
    if buy_notify is not None:
        meta_x._buy_notify = buy_notify
    if sell_notify is not None:
        meta_x._sell_notify = sell_notify
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# profitgoal
# ------------------------------------------------------------------
def crtPG(get_goal, calculate=None, params=None, name='crtPG', buy_notify=None, sell_notify=None):
    """
    Quickly create a profit goal strategy

    :param get_goal: the interface to get the goal price
    :param calculate: the internal calculation interface (called when the trading object is specified)
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param buy_notify: receive the notification of the buy trade records
    :param sell_notify: receive the notification of the sell trade records
    :return: the profit goal strategy instance
    """
    meta_x = type(name, (ProfitGoalBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x.get_goal = get_goal
    if calculate is not None:
        meta_x._calculate = calculate
    if buy_notify is not None:
        meta_x._buy_notify = buy_notify
    if sell_notify is not None:
        meta_x._sell_notify = sell_notify
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# signal
# ------------------------------------------------------------------
def crtSG(func, params=None, name='crtSG'):
    """
    Quickly create a signal generator

    :param func: the signal strategy function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom signal generator instance
    """
    meta_x = type(name, (SignalBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# multi_factor
# ------------------------------------------------------------------
def crtMF(calculate_func, params=None, name='crtMF'):
    """
    Quickly create a multi-factor composition algorithm

    :param calculate_func: the composition algorithm
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom multi-factor composition algorithm instance
    """
    meta_x = type(name, (MultiFactorBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._calculate = calculate_func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# slippage
# ------------------------------------------------------------------
def crtSP(get_real_buy_price, get_real_sell_price, params=None, name='crtSP', calculate=None):
    """
    Quickly create a slippage algorithm

    :param get_real_buy_price: the slippage algorithm interface to calculate the actual buy price
    :param get_real_sell_price: the slippage algorithm interface to calculate the actual sell price
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param calculate: the pre-processing function
    :return: the slippage algorithm instance
    """
    meta_x = type(name, (SlippageBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x.get_real_buy_price = get_real_buy_price
    meta_x.get_real_sell_price = get_real_sell_price
    if calculate is not None:
        meta_x._calculate = calculate
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# stoploss
# ------------------------------------------------------------------
def crtST(get_price, params=None, name='crtST', calculate=None, get_short_price=None):
    """
    Quickly create a stop-loss/take-profit strategy

    :param get_price: the interface function of the stop-loss/take-profit strategy to get the stop price
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param calculate: the initialization calculation function of the stop-loss/take-profit strategy
    :param get_short_price: the interface function to get the short stop price
    :return: the stop-loss/take-profit strategy instance
    """
    meta_x = type(name, (StoplossBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x.get_price = get_price
    if calculate is not None:
        meta_x._calculate = calculate
    if get_short_price is not None:
        meta_x.get_short_price = get_short_price
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# SCFilter
# ------------------------------------------------------------------
def crtSCFilter(filter_func, params=None, name='crtSCFilter'):
    """
    Quickly create a score filter

    :param filter_func: the score filter function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the score filter instance
    """
    meta_x = type(name, (ScoresFilterBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._filter = filter_func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret


# ------------------------------------------------------------------
# Normalize
# ------------------------------------------------------------------
def crtNorm(normalize_func, params=None, name='crtNorm'):
    """
    Quickly create algorithm functions such as standardization/normalization

    :param normalize_func: the algorithm function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the function instance
    """
    meta_x = type(name, (NormalizeBase, ), {'__init__': part_init, '_clone': part_clone})
    meta_x._normalize = normalize_func
    ret = meta_x(name, params)
    globals().update(dict(_=ret))
    return ret
