#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "operators/Indicator.h"
#include "operators/Indicator2InImp.h"

#define TA_IN1_OUT_DEF(func)                      \
  class Cls_##func : public IndicatorImp {        \
    INDICATOR_IMP(Cls_##func)                     \
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION \
                                                  \
   public:                                        \
    Cls_##func();                                 \
    virtual ~Cls_##func();                        \
  };

#define TA_IN1_OUT_DYN_DEF(func)                                 \
  class Cls_##func : public IndicatorImp {                       \
    INDICATOR_IMP(Cls_##func)                                    \
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE                          \
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION                \
                                                                 \
   public:                                                       \
    Cls_##func();                                                \
    virtual ~Cls_##func();                                       \
    virtual void _checkParam(const string &name) const override; \
  };

#define TA_IN2_OUT_DEF(func)                              \
  class Cls_##func : public Indicator2InImp {             \
    INDICATOR2IN_IMP(Cls_##func)                          \
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION      \
   public:                                                \
    Cls_##func();                                         \
    Cls_##func(const Indicator &ref_ind, bool fill_null); \
    virtual ~Cls_##func();                                \
  };

#define TA_IN2_OUT_N_DEF(func)                                   \
  class Cls_##func : public Indicator2InImp {                    \
    INDICATOR2IN_IMP(Cls_##func)                                 \
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION             \
   public:                                                       \
    Cls_##func();                                                \
    explicit Cls_##func(int n, bool fill_null);                  \
    Cls_##func(const Indicator &ref_ind, int n, bool fill_null); \
    virtual ~Cls_##func();                                       \
    virtual void _checkParam(const string &name) const override; \
  };

#define TA_K_OUT_DEF(func)                        \
  class Cls_##func : public IndicatorImp {        \
    INDICATOR_IMP(Cls_##func)                     \
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION \
   public:                                        \
    Cls_##func();                                 \
    explicit Cls_##func(const KData &);           \
    virtual ~Cls_##func() = default;              \
  };

#define TA_K_OUT_N_DEF(func)                                     \
  class Cls_##func : public IndicatorImp {                       \
    INDICATOR_IMP(Cls_##func)                                    \
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION                \
   public:                                                       \
    Cls_##func();                                                \
    explicit Cls_##func(const KData &, int n);                   \
    virtual ~Cls_##func() = default;                             \
    virtual void _checkParam(const string &name) const override; \
  };

#define TA_K_OUT_P_D_DEF(func)                                   \
  class Cls_##func : public IndicatorImp {                       \
    INDICATOR_IMP(Cls_##func)                                    \
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION                \
   public:                                                       \
    Cls_##func();                                                \
    explicit Cls_##func(const KData &, double p);                \
    virtual ~Cls_##func() = default;                             \
    virtual void _checkParam(const string &name) const override; \
  };

namespace hayaku {

TA_K_OUT_N_DEF(TA_ACCBANDS)
TA_IN1_OUT_DEF(TA_ACOS)
TA_K_OUT_DEF(TA_AD)
TA_IN2_OUT_DEF(TA_ADD)
TA_K_OUT_N_DEF(TA_ADX)
TA_K_OUT_N_DEF(TA_ADXR)
TA_K_OUT_N_DEF(TA_AROON)
TA_K_OUT_N_DEF(TA_AROONOSC)
TA_IN1_OUT_DEF(TA_ASIN)
TA_IN1_OUT_DEF(TA_ATAN)
TA_K_OUT_N_DEF(TA_ATR)
TA_IN1_OUT_DYN_DEF(TA_AVGDEV)
TA_K_OUT_DEF(TA_AVGPRICE)
TA_IN2_OUT_N_DEF(TA_BETA)
TA_K_OUT_DEF(TA_BOP)
TA_K_OUT_N_DEF(TA_CCI)
TA_K_OUT_DEF(TA_CDL2CROWS)
TA_K_OUT_DEF(TA_CDL3BLACKCROWS)
TA_K_OUT_DEF(TA_CDL3INSIDE)
TA_K_OUT_DEF(TA_CDL3LINESTRIKE)
TA_K_OUT_DEF(TA_CDL3OUTSIDE)
TA_K_OUT_DEF(TA_CDL3STARSINSOUTH)
TA_K_OUT_DEF(TA_CDL3WHITESOLDIERS)
TA_K_OUT_P_D_DEF(TA_CDLABANDONEDBABY)
TA_K_OUT_DEF(TA_CDLADVANCEBLOCK)
TA_K_OUT_DEF(TA_CDLBELTHOLD)
TA_K_OUT_DEF(TA_CDLBREAKAWAY)
TA_K_OUT_DEF(TA_CDLCLOSINGMARUBOZU)
TA_K_OUT_DEF(TA_CDLCONCEALBABYSWALL)
TA_K_OUT_DEF(TA_CDLCOUNTERATTACK)
TA_K_OUT_P_D_DEF(TA_CDLDARKCLOUDCOVER)
TA_K_OUT_DEF(TA_CDLDOJI)
TA_K_OUT_DEF(TA_CDLDOJISTAR)
TA_K_OUT_DEF(TA_CDLDRAGONFLYDOJI)
TA_K_OUT_DEF(TA_CDLENGULFING)
TA_K_OUT_P_D_DEF(TA_CDLEVENINGDOJISTAR)
TA_K_OUT_P_D_DEF(TA_CDLEVENINGSTAR)
TA_K_OUT_DEF(TA_CDLGAPSIDESIDEWHITE)
TA_K_OUT_DEF(TA_CDLGRAVESTONEDOJI)
TA_K_OUT_DEF(TA_CDLHAMMER)
TA_K_OUT_DEF(TA_CDLHANGINGMAN)
TA_K_OUT_DEF(TA_CDLHARAMI)
TA_K_OUT_DEF(TA_CDLHARAMICROSS)
TA_K_OUT_DEF(TA_CDLHIGHWAVE)
TA_K_OUT_DEF(TA_CDLHIKKAKE)
TA_K_OUT_DEF(TA_CDLHIKKAKEMOD)
TA_K_OUT_DEF(TA_CDLHOMINGPIGEON)
TA_K_OUT_DEF(TA_CDLIDENTICAL3CROWS)
TA_K_OUT_DEF(TA_CDLINNECK)
TA_K_OUT_DEF(TA_CDLINVERTEDHAMMER)
TA_K_OUT_DEF(TA_CDLKICKING)
TA_K_OUT_DEF(TA_CDLKICKINGBYLENGTH)
TA_K_OUT_DEF(TA_CDLLADDERBOTTOM)
TA_K_OUT_DEF(TA_CDLLONGLEGGEDDOJI)
TA_K_OUT_DEF(TA_CDLLONGLINE)
TA_K_OUT_DEF(TA_CDLMARUBOZU)
TA_K_OUT_DEF(TA_CDLMATCHINGLOW)
TA_K_OUT_P_D_DEF(TA_CDLMATHOLD)
TA_K_OUT_P_D_DEF(TA_CDLMORNINGDOJISTAR)
TA_K_OUT_P_D_DEF(TA_CDLMORNINGSTAR)
TA_K_OUT_DEF(TA_CDLONNECK)
TA_K_OUT_DEF(TA_CDLPIERCING)
TA_K_OUT_DEF(TA_CDLRICKSHAWMAN)
TA_K_OUT_DEF(TA_CDLRISEFALL3METHODS)
TA_K_OUT_DEF(TA_CDLSEPARATINGLINES)
TA_K_OUT_DEF(TA_CDLSHOOTINGSTAR)
TA_K_OUT_DEF(TA_CDLSHORTLINE)
TA_K_OUT_DEF(TA_CDLSPINNINGTOP)
TA_K_OUT_DEF(TA_CDLSTALLEDPATTERN)
TA_K_OUT_DEF(TA_CDLSTICKSANDWICH)
TA_K_OUT_DEF(TA_CDLTAKURI)
TA_K_OUT_DEF(TA_CDLTASUKIGAP)
TA_K_OUT_DEF(TA_CDLTHRUSTING)
TA_K_OUT_DEF(TA_CDLTRISTAR)
TA_K_OUT_DEF(TA_CDLUNIQUE3RIVER)
TA_K_OUT_DEF(TA_CDLUPSIDEGAP2CROWS)
TA_K_OUT_DEF(TA_CDLXSIDEGAP3METHODS)
TA_IN1_OUT_DEF(TA_CEIL)
TA_IN1_OUT_DYN_DEF(TA_CMO)
TA_IN2_OUT_N_DEF(TA_CORREL)
TA_IN1_OUT_DEF(TA_COS)
TA_IN1_OUT_DEF(TA_COSH)
TA_IN1_OUT_DYN_DEF(TA_DEMA)
TA_IN2_OUT_DEF(TA_DIV)
TA_K_OUT_N_DEF(TA_DX)
TA_IN1_OUT_DYN_DEF(TA_EMA)
TA_IN1_OUT_DEF(TA_EXP)
TA_IN1_OUT_DEF(TA_FLOOR)
TA_IN1_OUT_DEF(TA_HT_DCPERIOD)
TA_IN1_OUT_DEF(TA_HT_DCPHASE)
TA_IN1_OUT_DEF(TA_HT_PHASOR)
TA_IN1_OUT_DEF(TA_HT_SINE)
TA_IN1_OUT_DEF(TA_HT_TRENDLINE)
TA_IN1_OUT_DEF(TA_HT_TRENDMODE)
TA_K_OUT_N_DEF(TA_IMI)
TA_IN1_OUT_DYN_DEF(TA_KAMA)
TA_IN1_OUT_DYN_DEF(TA_LINEARREG_ANGLE)
TA_IN1_OUT_DYN_DEF(TA_LINEARREG_INTERCEPT)
TA_IN1_OUT_DYN_DEF(TA_LINEARREG_SLOPE)
TA_IN1_OUT_DYN_DEF(TA_LINEARREG)
TA_IN1_OUT_DEF(TA_LN)
TA_IN1_OUT_DEF(TA_LOG10)
TA_IN1_OUT_DYN_DEF(TA_MACDFIX)
TA_IN1_OUT_DYN_DEF(TA_MAX)
TA_IN1_OUT_DYN_DEF(TA_MAXINDEX)
TA_K_OUT_DEF(TA_MEDPRICE)
TA_K_OUT_N_DEF(TA_MFI)
TA_IN1_OUT_DYN_DEF(TA_MIDPOINT)
TA_K_OUT_N_DEF(TA_MIDPRICE)
TA_IN1_OUT_DYN_DEF(TA_MIN)
TA_IN1_OUT_DYN_DEF(TA_MININDEX)
TA_IN1_OUT_DYN_DEF(TA_MINMAX)
TA_IN1_OUT_DYN_DEF(TA_MINMAXINDEX)
TA_K_OUT_N_DEF(TA_MINUS_DI)
TA_K_OUT_N_DEF(TA_MINUS_DM)
TA_IN1_OUT_DYN_DEF(TA_MOM)
TA_IN2_OUT_DEF(TA_MULT)
TA_K_OUT_N_DEF(TA_NATR)
TA_K_OUT_DEF(TA_OBV)
TA_K_OUT_N_DEF(TA_PLUS_DI)
TA_K_OUT_N_DEF(TA_PLUS_DM)
TA_IN1_OUT_DYN_DEF(TA_ROC)
TA_IN1_OUT_DYN_DEF(TA_ROCP)
TA_IN1_OUT_DYN_DEF(TA_ROCR)
TA_IN1_OUT_DYN_DEF(TA_ROCR100)
TA_IN1_OUT_DYN_DEF(TA_RSI)
TA_IN1_OUT_DEF(TA_SIN)
TA_IN1_OUT_DEF(TA_SINH)
TA_IN1_OUT_DYN_DEF(TA_SMA)
TA_IN1_OUT_DEF(TA_SQRT)
TA_IN2_OUT_DEF(TA_SUB)
TA_IN1_OUT_DYN_DEF(TA_SUM)
TA_IN1_OUT_DEF(TA_TAN)
TA_IN1_OUT_DEF(TA_TANH)
TA_IN1_OUT_DYN_DEF(TA_TEMA)
TA_K_OUT_DEF(TA_TRANGE)
TA_IN1_OUT_DYN_DEF(TA_TRIMA)
TA_IN1_OUT_DYN_DEF(TA_TRIX)
TA_IN1_OUT_DYN_DEF(TA_TSF)
TA_K_OUT_DEF(TA_TYPPRICE)
TA_K_OUT_DEF(TA_WCLPRICE)
TA_K_OUT_N_DEF(TA_WILLR)
TA_IN1_OUT_DYN_DEF(TA_WMA)

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#define EXPOERT_TA_FUNC(func) BOOST_CLASS_EXPORT(hayaku::Cls_##func)

#define TA_IN1_OUT1_IMP(func, func_lookback)                     \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}           \
  Cls_##func::~Cls_##func() {}                                   \
                                                                 \
  void Cls_##func::_calculate(const Indicator &data) {           \
    int lookback = func_lookback();                              \
    size_t total = data.size();                                  \
    if (lookback < 0) {                                          \
      m_discard = total;                                         \
      return;                                                    \
    }                                                            \
    m_discard = data.discard() + lookback;                       \
    if (m_discard >= total) {                                    \
      m_discard = total;                                         \
      return;                                                    \
    }                                                            \
                                                                 \
    auto const *src = data.data();                               \
    auto *dst = this->data();                                    \
    int outBegIdx;                                               \
    int outNbElement;                                            \
    ::func(m_discard, total - 1, src, &outBegIdx, &outNbElement, \
           dst + m_discard);                                     \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                    \
                  (outBegIdx + outNbElement) <= total);          \
  }                                                              \
                                                                 \
  Indicator HAYAKU_API func() { return Indicator(make_shared<Cls_##func>()); }

#define TA_IN1_OUT1_INT_IMP(func, func_lookback)                             \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {}                       \
  Cls_##func::~Cls_##func() {}                                               \
                                                                             \
  void Cls_##func::_calculate(const Indicator &data) {                       \
    int lookback = func_lookback();                                          \
    size_t total = data.size();                                              \
    if (lookback < 0) {                                                      \
      m_discard = total;                                                     \
      return;                                                                \
    }                                                                        \
                                                                             \
    m_discard = data.discard() + lookback;                                   \
    if (m_discard >= total) {                                                \
      m_discard = total;                                                     \
      return;                                                                \
    }                                                                        \
                                                                             \
    auto const *src = data.data();                                           \
    std::unique_ptr<int[]> buf = std::make_unique<int[]>(total);             \
    int outBegIdx;                                                           \
    int outNbElement;                                                        \
    ::func(m_discard, total - 1, src, &outBegIdx, &outNbElement, buf.get()); \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                \
                  (outBegIdx + outNbElement) <= total);                      \
    m_discard = outBegIdx;                                                   \
    auto *dst = this->data();                                                \
    dst = dst + outBegIdx;                                                   \
    for (int i = 0; i < outNbElement; ++i) {                                 \
      dst[i] = buf[i];                                                       \
    }                                                                        \
  }                                                                          \
                                                                             \
  Indicator HAYAKU_API func() { return Indicator(make_shared<Cls_##func>()); }

#define TA_IN1_OUT1_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                          \
    setParam<int>("n", period);                                                \
  }                                                                            \
  Cls_##func::~Cls_##func() {}                                                 \
                                                                               \
  void Cls_##func::_checkParam(const string &name) const {                     \
    if (name == "n") {                                                         \
      int n = getParam<int>("n");                                              \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    int n = getParam<int>("n");                                                \
    int lookback = func_lookback(n);                                           \
    size_t total = data.size();                                                \
    if (lookback < 0) {                                                        \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    m_discard = data.discard() + lookback;                                     \
    if (m_discard >= total) {                                                  \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    auto const *src = data.data();                                             \
    auto *dst = this->data();                                                  \
                                                                               \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement,            \
           dst + m_discard);                                                   \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos,      \
                                     size_t step) {                            \
    int back = func_lookback(step);                                            \
    HAYAKU_IF_RETURN(back < 0 || back + ind.discard() > curPos, void());       \
                                                                               \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(curPos);        \
    auto const *src = ind.data();                                              \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement,        \
           buf.get());                                                         \
    if (outNbElement >= 1) {                                                   \
      _set(buf.get()[outNbElement - 1], curPos);                               \
    }                                                                          \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(int n) {                                           \
    auto p = make_shared<Cls_##func>();                                        \
    p->setParam<int>("n", n);                                                  \
    return Indicator(p);                                                       \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const IndParam &n) {                               \
    IndicatorImpPtr p = make_shared<Cls_##func>();                             \
    p->setIndParam("n", n);                                                    \
    return Indicator(p);                                                       \
  }

#define TA_IN1_OUT1_INT_N_IMP(func, func_lookback, period, period_min,    \
                              period_max)                                 \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                     \
    setParam<int>("n", period);                                           \
  }                                                                       \
  Cls_##func::~Cls_##func() {}                                            \
                                                                          \
  void Cls_##func::_checkParam(const string &name) const {                \
    if (name == "n") {                                                    \
      int n = getParam<int>("n");                                         \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                  \
    }                                                                     \
  }                                                                       \
                                                                          \
  void Cls_##func::_calculate(const Indicator &data) {                    \
    int n = getParam<int>("n");                                           \
    int lookback = func_lookback(n);                                      \
    size_t total = data.size();                                           \
    if (lookback < 0) {                                                   \
      m_discard = total;                                                  \
      return;                                                             \
    }                                                                     \
                                                                          \
    m_discard = data.discard() + lookback;                                \
    if (m_discard >= total) {                                             \
      m_discard = total;                                                  \
      return;                                                             \
    }                                                                     \
                                                                          \
    auto const *src = data.data();                                        \
    std::unique_ptr<int[]> buf = std::make_unique<int[]>(total);          \
    int outBegIdx;                                                        \
    int outNbElement;                                                     \
    ::func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement,       \
           buf.get());                                                    \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                             \
                  (outBegIdx + outNbElement) <= total);                   \
    m_discard = outBegIdx;                                                \
    auto *dst = this->data();                                             \
    dst = dst + outBegIdx;                                                \
    for (int i = 0; i < outNbElement; ++i) {                              \
      dst[i] = buf[i];                                                    \
    }                                                                     \
  }                                                                       \
                                                                          \
  void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos, \
                                     size_t step) {                       \
    int back = func_lookback(step);                                       \
    HAYAKU_IF_RETURN(back < 0 || back + ind.discard() > curPos, void());  \
                                                                          \
    std::unique_ptr<int[]> buf = std::make_unique<int[]>(curPos);         \
    auto const *src = ind.data();                                         \
    int outBegIdx;                                                        \
    int outNbElement;                                                     \
    ::func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement,   \
           buf.get());                                                    \
    if (outNbElement >= 1) {                                              \
      _set(buf.get()[outNbElement - 1], curPos);                          \
    }                                                                     \
  }                                                                       \
                                                                          \
  Indicator HAYAKU_API func(int n) {                                      \
    auto p = make_shared<Cls_##func>();                                   \
    p->setParam<int>("n", n);                                             \
    return Indicator(p);                                                  \
  }                                                                       \
                                                                          \
  Indicator HAYAKU_API func(const IndParam &n) {                          \
    IndicatorImpPtr p = make_shared<Cls_##func>();                        \
    p->setIndParam("n", n);                                               \
    return Indicator(p);                                                  \
  }

#define TA_IN1_OUT2_IMP(func, func_lookback)                     \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {}           \
  Cls_##func::~Cls_##func() {}                                   \
  void Cls_##func::_calculate(const Indicator &data) {           \
    size_t total = data.size();                                  \
    int lookback = func_lookback();                              \
    if (lookback < 0) {                                          \
      m_discard = total;                                         \
      return;                                                    \
    }                                                            \
    m_discard = data.discard() + lookback;                       \
    if (m_discard >= total) {                                    \
      m_discard = total;                                         \
      return;                                                    \
    }                                                            \
                                                                 \
    auto const *src = data.data();                               \
    auto *dst0 = this->data(0);                                  \
    auto *dst1 = this->data(1);                                  \
                                                                 \
    int outBegIdx;                                               \
    int outNbElement;                                            \
    ::func(m_discard, total - 1, src, &outBegIdx, &outNbElement, \
           dst0 + m_discard, dst1 + m_discard);                  \
    HAYAKU_ASSERT(outBegIdx == m_discard &&                      \
                  (outBegIdx + outNbElement) <= total);          \
  }                                                              \
                                                                 \
  Indicator HAYAKU_API func() { return Indicator(make_shared<Cls_##func>()); }

#define TA_IN1_OUT2_INT_N_IMP(func, func_lookback, period, period_min,        \
                              period_max)                                     \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {                         \
    setParam<int>("n", period);                                               \
  }                                                                           \
  Cls_##func::~Cls_##func() {}                                                \
                                                                              \
  void Cls_##func::_checkParam(const string &name) const {                    \
    if (name == "n") {                                                        \
      int n = getParam<int>("n");                                             \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  void Cls_##func::_calculate(const Indicator &data) {                        \
    int n = getParam<int>("n");                                               \
    int lookback = func_lookback(n);                                          \
    size_t total = data.size();                                               \
    if (lookback < 0) {                                                       \
      m_discard = total;                                                      \
      return;                                                                 \
    }                                                                         \
                                                                              \
    m_discard = data.discard() + lookback;                                    \
    if (m_discard >= total) {                                                 \
      m_discard = total;                                                      \
      return;                                                                 \
    }                                                                         \
                                                                              \
    auto const *src = data.data();                                            \
    std::unique_ptr<int[]> buf = std::make_unique<int[]>(2 * (total));        \
    int *buf0 = buf.get();                                                    \
    int *buf1 = buf0 + total;                                                 \
    int outBegIdx;                                                            \
    int outNbElement;                                                         \
    ::func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement, buf0,     \
           buf1);                                                             \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                 \
                  (outBegIdx + outNbElement) <= total);                       \
    m_discard = outBegIdx;                                                    \
    auto *dst0 = this->data(0) + outBegIdx;                                   \
    auto *dst1 = this->data(1) + outBegIdx;                                   \
    for (int i = 0; i < outNbElement; ++i) {                                  \
      dst0[i] = buf0[i];                                                      \
      dst1[i] = buf1[i];                                                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos,     \
                                     size_t step) {                           \
    int back = func_lookback(step);                                           \
    HAYAKU_IF_RETURN(back < 0 || back + ind.discard() > curPos, void());      \
                                                                              \
    std::unique_ptr<int[]> buf = std::make_unique<int[]>(2 * curPos);         \
    int *buf0 = buf.get();                                                    \
    int *buf1 = buf0 + curPos;                                                \
    auto const *src = ind.data();                                             \
    int outBegIdx;                                                            \
    int outNbElement;                                                         \
    ::func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, buf0, \
           buf1);                                                             \
    if (outNbElement >= 1) {                                                  \
      _set(buf0[outNbElement - 1], curPos, 0);                                \
      _set(buf1[outNbElement - 1], curPos, 1);                                \
    }                                                                         \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(int n) {                                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    return Indicator(p);                                                      \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(const IndParam &n) {                              \
    IndicatorImpPtr p = make_shared<Cls_##func>();                            \
    p->setIndParam("n", n);                                                   \
    return Indicator(p);                                                      \
  }

#define TA_IN1_OUT2_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {                          \
    setParam<int>("n", period);                                                \
  }                                                                            \
  Cls_##func::~Cls_##func() {}                                                 \
                                                                               \
  void Cls_##func::_checkParam(const string &name) const {                     \
    if (name == "n") {                                                         \
      int n = getParam<int>("n");                                              \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    int n = getParam<int>("n");                                                \
    size_t total = data.size();                                                \
    int lookback = func_lookback(n);                                           \
    if (lookback < 0) {                                                        \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
    m_discard = data.discard() + lookback;                                     \
    if (m_discard >= total) {                                                  \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    auto const *src = data.data();                                             \
    auto *dst0 = this->data(0);                                                \
    auto *dst1 = this->data(1);                                                \
                                                                               \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement,            \
           dst0 + m_discard, dst1 + m_discard);                                \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos,      \
                                     size_t step) {                            \
    int back = func_lookback(step);                                            \
    HAYAKU_IF_RETURN(back < 0 || back + ind.discard() > curPos, void());       \
                                                                               \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * curPos);    \
    double *dst0 = buf.get();                                                  \
    double *ds1 = dst0 + curPos;                                               \
    auto const *src = ind.data();                                              \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, dst0,  \
           ds1);                                                               \
    if (outNbElement >= 1) {                                                   \
      _set(dst0[outNbElement - 1], curPos, 0);                                 \
      _set(ds1[outNbElement - 1], curPos, 1);                                  \
    }                                                                          \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(int n) {                                           \
    auto p = make_shared<Cls_##func>();                                        \
    p->setParam<int>("n", n);                                                  \
    return Indicator(p);                                                       \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const IndParam &n) {                               \
    IndicatorImpPtr p = make_shared<Cls_##func>();                             \
    p->setIndParam("n", n);                                                    \
    return Indicator(p);                                                       \
  }

#define TA_IN1_OUT3_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 3) {                          \
    setParam<int>("n", period);                                                \
  }                                                                            \
  Cls_##func::~Cls_##func() {}                                                 \
                                                                               \
  void Cls_##func::_checkParam(const string &name) const {                     \
    if (name == "n") {                                                         \
      int n = getParam<int>("n");                                              \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    int n = getParam<int>("n");                                                \
    size_t total = data.size();                                                \
    int lookback = func_lookback(n);                                           \
    if (lookback < 0) {                                                        \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
    m_discard = data.discard() + lookback;                                     \
    if (m_discard >= total) {                                                  \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    auto const *src = data.data();                                             \
    auto *dst0 = this->data(0);                                                \
    auto *dst1 = this->data(1);                                                \
    auto *dst2 = this->data(2);                                                \
                                                                               \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(m_discard, total - 1, src, n, &outBegIdx, &outNbElement,            \
           dst0 + m_discard, dst1 + m_discard, dst2 + m_discard);              \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  void Cls_##func::_dyn_run_one_step(const Indicator &ind, size_t curPos,      \
                                     size_t step) {                            \
    int back = func_lookback(step);                                            \
    HAYAKU_IF_RETURN(back < 0 || back + ind.discard() > curPos, void());       \
                                                                               \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * curPos);    \
    double *dst0 = buf.get();                                                  \
    double *ds1 = dst0 + curPos;                                               \
    double *ds2 = ds1 + curPos;                                                \
    auto const *src = ind.data();                                              \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(ind.discard(), curPos, src, step, &outBegIdx, &outNbElement, dst0,  \
           ds1, ds2);                                                          \
    if (outNbElement >= 1) {                                                   \
      _set(dst0[outNbElement - 1], curPos, 0);                                 \
      _set(ds1[outNbElement - 1], curPos, 1);                                  \
      _set(ds2[outNbElement - 1], curPos, 2);                                  \
    }                                                                          \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(int n) {                                           \
    auto p = make_shared<Cls_##func>();                                        \
    p->setParam<int>("n", n);                                                  \
    return Indicator(p);                                                       \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const IndParam &n) {                               \
    IndicatorImpPtr p = make_shared<Cls_##func>();                             \
    p->setIndParam("n", n);                                                    \
    return Indicator(p);                                                       \
  }

#define TA_IN2_OUT1_IMP(func, func_lookback)                              \
  Cls_##func::Cls_##func() : Indicator2InImp(#func, 1) {}                 \
  Cls_##func::Cls_##func(const Indicator &ref_ind, bool fill_null)        \
      : Indicator2InImp(#func, ref_ind, fill_null, 1) {}                  \
  Cls_##func::~Cls_##func() {}                                            \
                                                                          \
  void Cls_##func::_calculate(const Indicator &ind) {                     \
    size_t total = ind.size();                                            \
    HAYAKU_IF_RETURN(total == 0, void());                                 \
                                                                          \
    Indicator ref = prepare(ind);                                         \
    int lookback = func_lookback();                                       \
    if (lookback < 0) {                                                   \
      m_discard = total;                                                  \
      return;                                                             \
    }                                                                     \
                                                                          \
    size_t in_discard = std::max(ind.discard(), ref.discard());           \
    m_discard = lookback + in_discard;                                    \
    if (m_discard >= total) {                                             \
      m_discard = total;                                                  \
      return;                                                             \
    }                                                                     \
                                                                          \
    const auto *src0 = ind.data();                                        \
    const auto *src1 = ref.data();                                        \
    auto *dst = this->data();                                             \
    int outBegIdx;                                                        \
    int outNbElement;                                                     \
    ::func(m_discard, total - 1, src0, src1, &outBegIdx, &outNbElement,   \
           dst + m_discard);                                              \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                             \
                  (outBegIdx + outNbElement) <= total);                   \
  }                                                                       \
                                                                          \
  Indicator HAYAKU_API func(bool fill_null) {                             \
    auto p = make_shared<Cls_##func>();                                   \
    p->setParam<bool>("fill_null", fill_null);                            \
    return Indicator(p);                                                  \
  }                                                                       \
                                                                          \
  Indicator HAYAKU_API func(const Indicator &ind1, const Indicator &ind2, \
                            bool fill_null) {                             \
    auto p = make_shared<Cls_##func>(ind2, fill_null);                    \
    Indicator result(p);                                                  \
    return result(ind1);                                                  \
  }

#define TA_IN2_OUT1_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : Indicator2InImp(#func, 1) {                       \
    setParam<int>("n", period);                                                \
  }                                                                            \
                                                                               \
  Cls_##func::Cls_##func(int n, bool fill_null) : Indicator2InImp(#func, 1) {  \
    setParam<int>("n", n);                                                     \
    setParam<bool>("fill_null", fill_null);                                    \
  }                                                                            \
                                                                               \
  Cls_##func::Cls_##func(const Indicator &ref_ind, int n, bool fill_null)      \
      : Indicator2InImp(#func, ref_ind, fill_null, 1) {                        \
    setParam<int>("n", n);                                                     \
  }                                                                            \
                                                                               \
  Cls_##func::~Cls_##func() {}                                                 \
                                                                               \
  void Cls_##func::_checkParam(const string &name) const {                     \
    if ("n" == name) {                                                         \
      int n = getParam<int>("n");                                              \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Cls_##func::_calculate(const Indicator &ind) {                          \
    size_t total = ind.size();                                                 \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    Indicator ref = prepare(ind);                                              \
                                                                               \
    int n = getParam<int>("n");                                                \
    int lookback = func_lookback(n);                                           \
    if (lookback < 0) {                                                        \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    size_t in_discard = std::max(ind.discard(), ref.discard());                \
    m_discard = lookback + in_discard;                                         \
    if (m_discard >= total) {                                                  \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const auto *src0 = ind.data();                                             \
    const auto *src1 = ref.data();                                             \
    auto *dst = this->data();                                                  \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(m_discard, total - 1, src0, src1, n, &outBegIdx, &outNbElement,     \
           dst + m_discard);                                                   \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(int n, bool fill_null) {                           \
    return Indicator(make_shared<Cls_##func>(n, fill_null));                   \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const Indicator &ind1, const Indicator &ind2,      \
                            int n, bool fill_null) {                           \
    auto p = make_shared<Cls_##func>(ind2, n, fill_null);                      \
    Indicator result(p);                                                       \
    return result(ind1);                                                       \
  }

#define TA_OHLC_OUT1_IMP(func, func_lookback)                                  \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) { m_need_context = true; } \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                 \
                   "The input is ignored because {} depends on the context!",  \
                   m_name);                                                    \
                                                                               \
    KData k = getContext();                                                    \
    size_t total = k.size();                                                   \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    _readyBuffer(total, 1);                                                    \
    int lookback = func_lookback();                                            \
    if (lookback < 0 || lookback >= total) {                                   \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const KRecord *kptr = k.data();                                            \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);     \
    double *open = buf.get();                                                  \
    double *high = open + total;                                               \
    double *low = high + total;                                                \
    double *close = low + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                       \
      open[i] = kptr[i].openPrice;                                             \
      high[i] = kptr[i].highPrice;                                             \
      low[i] = kptr[i].lowPrice;                                               \
      close[i] = kptr[i].closePrice;                                           \
    }                                                                          \
                                                                               \
    m_discard = lookback;                                                      \
    auto *dst = this->data();                                                  \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(m_discard, total - 1, open, high, low, close, &outBegIdx,           \
           &outNbElement, dst + m_discard);                                    \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func() {                                                \
    return make_shared<Cls_##func>()->calculate();                             \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const KData &k) {                                  \
    auto p = make_shared<Cls_##func>();                                        \
    p->setContext(k);                                                          \
    return Indicator(p);                                                       \
  }

#define TA_OHLC_OUT1_INT_IMP(func, func_lookback)                              \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) { m_need_context = true; } \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                 \
                   "The input is ignored because {} depends on the context!",  \
                   m_name);                                                    \
                                                                               \
    KData k = getContext();                                                    \
    size_t total = k.size();                                                   \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    _readyBuffer(total, 1);                                                    \
                                                                               \
    int lookback = func_lookback();                                            \
    if (lookback < 0 || lookback >= total) {                                   \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const KRecord *kptr = k.data();                                            \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);     \
    double *open = buf.get();                                                  \
    double *high = open + total;                                               \
    double *low = high + total;                                                \
    double *close = low + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                       \
      open[i] = kptr[i].openPrice;                                             \
      high[i] = kptr[i].highPrice;                                             \
      low[i] = kptr[i].lowPrice;                                               \
      close[i] = kptr[i].closePrice;                                           \
    }                                                                          \
                                                                               \
    std::unique_ptr<int[]> outbuf = std::make_unique<int[]>(total);            \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    m_discard = lookback;                                                      \
    ::func(m_discard, total - 1, open, high, low, close, &outBegIdx,           \
           &outNbElement, outbuf.get());                                       \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
    auto *dst = this->data() + outBegIdx;                                      \
    for (size_t i = 0; i < outNbElement; ++i) {                                \
      dst[i] = outbuf[i];                                                      \
    }                                                                          \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func() {                                                \
    return make_shared<Cls_##func>()->calculate();                             \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const KData &k) {                                  \
    auto p = make_shared<Cls_##func>();                                        \
    p->setContext(k);                                                          \
    return Indicator(p);                                                       \
  }

#define TA_OHLC_OUT1_INT_P1_D_IMP(func, func_lookback, param1, param1_value,  \
                                  param1_min, param1_max)                     \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                         \
    m_need_context = true;                                                    \
    setParam<double>(#param1, param1_value);                                  \
  }                                                                           \
                                                                              \
  void Cls_##func::_checkParam(const string &name) const {                    \
    if (name == #param1) {                                                    \
      double p = getParam<double>(#param1);                                   \
      HAYAKU_ASSERT(p >= param1_min && p <= param1_max);                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  void Cls_##func::_calculate(const Indicator &data) {                        \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                \
                   "The input is ignored because {} depends on the context!", \
                   m_name);                                                   \
                                                                              \
    KData k = getContext();                                                   \
    size_t total = k.size();                                                  \
    HAYAKU_IF_RETURN(total == 0, void());                                     \
                                                                              \
    _readyBuffer(total, 1);                                                   \
                                                                              \
    int lookback = func_lookback(param1_value);                               \
    if (lookback < 0 || lookback >= total) {                                  \
      m_discard = total;                                                      \
      return;                                                                 \
    }                                                                         \
                                                                              \
    const KRecord *kptr = k.data();                                           \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);    \
    double *open = buf.get();                                                 \
    double *high = open + total;                                              \
    double *low = high + total;                                               \
    double *close = low + total;                                              \
    for (size_t i = 0; i < total; ++i) {                                      \
      open[i] = kptr[i].openPrice;                                            \
      high[i] = kptr[i].highPrice;                                            \
      low[i] = kptr[i].lowPrice;                                              \
      close[i] = kptr[i].closePrice;                                          \
    }                                                                         \
                                                                              \
    std::unique_ptr<int[]> outbuf = std::make_unique<int[]>(total);           \
    int outBegIdx;                                                            \
    int outNbElement;                                                         \
    m_discard = lookback;                                                     \
    ::func(m_discard, total - 1, open, high, low, close,                      \
           getParam<double>(#param1), &outBegIdx, &outNbElement,              \
           outbuf.get());                                                     \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                 \
                  (outBegIdx + outNbElement) <= total);                       \
    auto *dst = this->data() + outBegIdx;                                     \
    for (size_t i = 0; i < outNbElement; ++i) {                               \
      dst[i] = outbuf[i];                                                     \
    }                                                                         \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(double p) {                                       \
    auto ptr = make_shared<Cls_##func>();                                     \
    ptr->setParam<double>(#param1, p);                                        \
    ptr->calculate();                                                         \
    return Indicator(ptr);                                                    \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(const KData &k, double p) {                       \
    auto ptr = make_shared<Cls_##func>();                                     \
    ptr->setParam<double>(#param1, p);                                        \
    ptr->setContext(k);                                                       \
    return Indicator(ptr);                                                    \
  }

#define TA_HLCV_OUT1_IMP(func, func_lookback)                                  \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) { m_need_context = true; } \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                 \
                   "The input is ignored because {} depends on the context!",  \
                   m_name);                                                    \
                                                                               \
    KData k = getContext();                                                    \
    size_t total = k.size();                                                   \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    _readyBuffer(total, 1);                                                    \
    int lookback = func_lookback();                                            \
    if (lookback < 0 || lookback >= total) {                                   \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const KRecord *kptr = k.data();                                            \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);     \
    double *high = buf.get();                                                  \
    double *low = high + total;                                                \
    double *close = low + total;                                               \
    double *vol = close + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                       \
      high[i] = kptr[i].highPrice;                                             \
      low[i] = kptr[i].lowPrice;                                               \
      close[i] = kptr[i].closePrice;                                           \
      vol[i] = kptr[i].transCount;                                             \
    }                                                                          \
                                                                               \
    auto *dst = this->data();                                                  \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    m_discard = lookback;                                                      \
    ::func(m_discard, total - 1, high, low, close, vol, &outBegIdx,            \
           &outNbElement, dst + m_discard);                                    \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func() {                                                \
    return make_shared<Cls_##func>()->calculate();                             \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const KData &k) {                                  \
    auto ptr = make_shared<Cls_##func>();                                      \
    ptr->setContext(k);                                                        \
    return Indicator(ptr);                                                     \
  }

#define TA_HL_OUT1_IMP(func, func_lookback)                                    \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) { m_need_context = true; } \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                 \
                   "The input is ignored because {} depends on the context!",  \
                   m_name);                                                    \
                                                                               \
    KData k = getContext();                                                    \
    size_t total = k.size();                                                   \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    _readyBuffer(total, 1);                                                    \
    int lookback = func_lookback();                                            \
    if (lookback < 0 || lookback >= total) {                                   \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const KRecord *kptr = k.data();                                            \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);     \
    double *high = buf.get();                                                  \
    double *low = high + total;                                                \
    for (size_t i = 0; i < total; ++i) {                                       \
      high[i] = kptr[i].highPrice;                                             \
      low[i] = kptr[i].lowPrice;                                               \
    }                                                                          \
                                                                               \
    auto *dst = this->data();                                                  \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    m_discard = lookback;                                                      \
    ::func(m_discard, total - 1, high, low, &outBegIdx, &outNbElement,         \
           dst + m_discard);                                                   \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func() {                                                \
    return make_shared<Cls_##func>()->calculate();                             \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const KData &k) {                                  \
    auto ptr = make_shared<Cls_##func>();                                      \
    ptr->setContext(k);                                                        \
    return Indicator(ptr);                                                     \
  }

#define TA_CV_OUT1_IMP(func, func_lookback)                                    \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) { m_need_context = true; } \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                 \
                   "The input is ignored because {} depends on the context!",  \
                   m_name);                                                    \
                                                                               \
    KData k = getContext();                                                    \
    size_t total = k.size();                                                   \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    _readyBuffer(total, 1);                                                    \
    int lookback = func_lookback();                                            \
    if (lookback < 0 || lookback >= total) {                                   \
      m_discard = 0;                                                           \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const KRecord *kptr = k.data();                                            \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);     \
    double *close = buf.get();                                                 \
    double *vol = close + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                       \
      close[i] = kptr[i].closePrice;                                           \
      vol[i] = kptr[i].transCount;                                             \
    }                                                                          \
                                                                               \
    auto *dst = this->data();                                                  \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    m_discard = lookback;                                                      \
    ::func(m_discard, total - 1, close, vol, &outBegIdx, &outNbElement,        \
           dst + m_discard);                                                   \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func() {                                                \
    return make_shared<Cls_##func>()->calculate();                             \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const KData &k) {                                  \
    auto ptr = make_shared<Cls_##func>();                                      \
    ptr->setContext(k);                                                        \
    return Indicator(ptr);                                                     \
  }

#define TA_HLC_OUT1_IMP(func, func_lookback)                                   \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) { m_need_context = true; } \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                 \
                   "The input is ignored because {} depends on the context!",  \
                   m_name);                                                    \
                                                                               \
    KData k = getContext();                                                    \
    size_t total = k.size();                                                   \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    _readyBuffer(total, 1);                                                    \
    int lookback = func_lookback();                                            \
    if (lookback < 0 || lookback >= total) {                                   \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const KRecord *kptr = k.data();                                            \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);     \
    double *high = buf.get();                                                  \
    double *low = high + total;                                                \
    double *close = low + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                       \
      high[i] = kptr[i].highPrice;                                             \
      low[i] = kptr[i].lowPrice;                                               \
      close[i] = kptr[i].closePrice;                                           \
    }                                                                          \
                                                                               \
    m_discard = lookback;                                                      \
    auto *dst = this->data();                                                  \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(m_discard, total - 1, high, low, close, &outBegIdx, &outNbElement,  \
           dst + m_discard);                                                   \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func() {                                                \
    return make_shared<Cls_##func>()->calculate();                             \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const KData &k) {                                  \
    auto ptr = make_shared<Cls_##func>();                                      \
    ptr->setContext(k);                                                        \
    return Indicator(ptr);                                                     \
  }

#define TA_HLC_OUT1_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                          \
    m_need_context = true;                                                     \
    setParam<int>("n", period);                                                \
  }                                                                            \
                                                                               \
  void Cls_##func::_checkParam(const string &name) const {                     \
    if (name == "n") {                                                         \
      int n = getParam<int>("n");                                              \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                 \
                   "The input is ignored because {} depends on the context!",  \
                   m_name);                                                    \
                                                                               \
    KData k = getContext();                                                    \
    size_t total = k.size();                                                   \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    _readyBuffer(total, 1);                                                    \
    int n = getParam<int>("n");                                                \
    int back = func_lookback(n);                                               \
    if (back < 0 || back >= total) {                                           \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const KRecord *kptr = k.data();                                            \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);     \
    double *high = buf.get();                                                  \
    double *low = high + total;                                                \
    double *close = low + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                       \
      high[i] = kptr[i].highPrice;                                             \
      low[i] = kptr[i].lowPrice;                                               \
      close[i] = kptr[i].closePrice;                                           \
    }                                                                          \
                                                                               \
    m_discard = back;                                                          \
    auto *dst = this->data();                                                  \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(m_discard, total - 1, high, low, close, n, &outBegIdx,              \
           &outNbElement, dst + m_discard);                                    \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(int n) {                                           \
    auto p = make_shared<Cls_##func>();                                        \
    p->setParam<int>("n", n);                                                  \
    p->calculate();                                                            \
    return Indicator(p);                                                       \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const KData &k, int n) {                           \
    auto p = make_shared<Cls_##func>();                                        \
    p->setParam<int>("n", n);                                                  \
    p->setContext(k);                                                          \
    return Indicator(p);                                                       \
  }

#define TA_HLCV_OUT1_N_IMP(func, func_lookback, period, period_min,           \
                           period_max)                                        \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                         \
    m_need_context = true;                                                    \
    setParam<int>("n", period);                                               \
  }                                                                           \
                                                                              \
  void Cls_##func::_checkParam(const string &name) const {                    \
    if (name == "n") {                                                        \
      int n = getParam<int>("n");                                             \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  void Cls_##func::_calculate(const Indicator &data) {                        \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                \
                   "The input is ignored because {} depends on the context!", \
                   m_name);                                                   \
                                                                              \
    KData k = getContext();                                                   \
    size_t total = k.size();                                                  \
    HAYAKU_IF_RETURN(total == 0, void());                                     \
                                                                              \
    _readyBuffer(total, 1);                                                   \
                                                                              \
    int n = getParam<int>("n");                                               \
    int back = func_lookback(n);                                              \
    if (back < 0 || back >= total) {                                          \
      m_discard = total;                                                      \
      return;                                                                 \
    }                                                                         \
                                                                              \
    const KRecord *kptr = k.data();                                           \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);    \
    double *high = buf.get();                                                 \
    double *low = high + total;                                               \
    double *close = low + total;                                              \
    double *vol = close + total;                                              \
    for (size_t i = 0; i < total; ++i) {                                      \
      high[i] = kptr[i].highPrice;                                            \
      low[i] = kptr[i].lowPrice;                                              \
      close[i] = kptr[i].closePrice;                                          \
      vol[i] = kptr[i].transCount;                                            \
    }                                                                         \
                                                                              \
    m_discard = back;                                                         \
    auto *dst = this->data();                                                 \
    int outBegIdx;                                                            \
    int outNbElement;                                                         \
    ::func(m_discard, total - 1, high, low, close, vol, n, &outBegIdx,        \
           &outNbElement, dst + m_discard);                                   \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                 \
                  (outBegIdx + outNbElement) <= total);                       \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(int n) {                                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    p->calculate();                                                           \
    return Indicator(p);                                                      \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(const KData &k, int n) {                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    p->setContext(k);                                                         \
    return Indicator(p);                                                      \
  }

#define TA_HL_OUT1_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                         \
    m_need_context = true;                                                    \
    setParam<int>("n", period);                                               \
  }                                                                           \
                                                                              \
  void Cls_##func::_checkParam(const string &name) const {                    \
    if (name == "n") {                                                        \
      int n = getParam<int>("n");                                             \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  void Cls_##func::_calculate(const Indicator &data) {                        \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                \
                   "The input is ignored because {} depends on the context!", \
                   m_name);                                                   \
                                                                              \
    KData k = getContext();                                                   \
    size_t total = k.size();                                                  \
    HAYAKU_IF_RETURN(total == 0, void());                                     \
                                                                              \
    _readyBuffer(total, 1);                                                   \
    int n = getParam<int>("n");                                               \
    int back = func_lookback(n);                                              \
    if (back < 0 || back >= total) {                                          \
      m_discard = total;                                                      \
      return;                                                                 \
    }                                                                         \
                                                                              \
    const KRecord *kptr = k.data();                                           \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);    \
    double *high = buf.get();                                                 \
    double *low = high + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                      \
      high[i] = kptr[i].highPrice;                                            \
      low[i] = kptr[i].lowPrice;                                              \
    }                                                                         \
                                                                              \
    m_discard = back;                                                         \
    auto *dst = this->data();                                                 \
    int outBegIdx;                                                            \
    int outNbElement;                                                         \
    ::func(m_discard, total - 1, high, low, n, &outBegIdx, &outNbElement,     \
           dst + m_discard);                                                  \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                 \
                  (outBegIdx + outNbElement) <= total);                       \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(int n) {                                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    p->calculate();                                                           \
    return Indicator(p);                                                      \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(const KData &k, int n) {                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    p->setContext(k);                                                         \
    return Indicator(p);                                                      \
  }

#define TA_HL_OUT2_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 2) {                         \
    m_need_context = true;                                                    \
    setParam<int>("n", period);                                               \
  }                                                                           \
                                                                              \
  void Cls_##func::_checkParam(const string &name) const {                    \
    if (name == "n") {                                                        \
      int n = getParam<int>("n");                                             \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  void Cls_##func::_calculate(const Indicator &data) {                        \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                \
                   "The input is ignored because {} depends on the context!", \
                   m_name);                                                   \
                                                                              \
    KData k = getContext();                                                   \
    size_t total = k.size();                                                  \
    HAYAKU_IF_RETURN(total == 0, void());                                     \
                                                                              \
    _readyBuffer(total, 2);                                                   \
    int n = getParam<int>("n");                                               \
    int back = func_lookback(n);                                              \
    if (back < 0 || back >= total) {                                          \
      m_discard = total;                                                      \
      return;                                                                 \
    }                                                                         \
                                                                              \
    const KRecord *kptr = k.data();                                           \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);    \
    double *high = buf.get();                                                 \
    double *low = high + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                      \
      high[i] = kptr[i].highPrice;                                            \
      low[i] = kptr[i].lowPrice;                                              \
    }                                                                         \
                                                                              \
    m_discard = back;                                                         \
    auto *dst0 = this->data(0);                                               \
    auto *dst1 = this->data(1);                                               \
    int outBegIdx;                                                            \
    int outNbElement;                                                         \
    ::func(m_discard, total - 1, high, low, n, &outBegIdx, &outNbElement,     \
           dst0 + m_discard, dst1 + m_discard);                               \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                 \
                  (outBegIdx + outNbElement) <= total);                       \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(int n) {                                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    p->calculate();                                                           \
    return Indicator(p);                                                      \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(const KData &k, int n) {                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    p->setContext(k);                                                         \
    return Indicator(p);                                                      \
  }

#define TA_HLC_OUT3_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 3) {                          \
    m_need_context = true;                                                     \
    setParam<int>("n", period);                                                \
  }                                                                            \
                                                                               \
  void Cls_##func::_checkParam(const string &name) const {                     \
    if (name == "n") {                                                         \
      int n = getParam<int>("n");                                              \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Cls_##func::_calculate(const Indicator &data) {                         \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                 \
                   "The input is ignored because {} depends on the context!",  \
                   m_name);                                                    \
                                                                               \
    KData k = getContext();                                                    \
    size_t total = k.size();                                                   \
    HAYAKU_IF_RETURN(total == 0, void());                                      \
                                                                               \
    _readyBuffer(total, 3);                                                    \
    int n = getParam<int>("n");                                                \
    int back = func_lookback(n);                                               \
    if (back < 0 || back >= total) {                                           \
      m_discard = total;                                                       \
      return;                                                                  \
    }                                                                          \
                                                                               \
    const KRecord *kptr = k.data();                                            \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);     \
    double *high = buf.get();                                                  \
    double *low = high + total;                                                \
    double *close = low + total;                                               \
    for (size_t i = 0; i < total; ++i) {                                       \
      high[i] = kptr[i].highPrice;                                             \
      low[i] = kptr[i].lowPrice;                                               \
      close[i] = kptr[i].closePrice;                                           \
    }                                                                          \
                                                                               \
    auto *dst0 = this->data(0);                                                \
    auto *dst1 = this->data(1);                                                \
    auto *dst2 = this->data(2);                                                \
    m_discard = back;                                                          \
    int outBegIdx;                                                             \
    int outNbElement;                                                          \
    ::func(m_discard, total - 1, high, low, close, n, &outBegIdx,              \
           &outNbElement, dst0 + m_discard, dst1 + m_discard,                  \
           dst2 + m_discard);                                                  \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                  \
                  (outBegIdx + outNbElement) <= total);                        \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(int n) {                                           \
    auto p = make_shared<Cls_##func>();                                        \
    p->setParam<int>("n", n);                                                  \
    p->calculate();                                                            \
    return Indicator(p);                                                       \
  }                                                                            \
                                                                               \
  Indicator HAYAKU_API func(const KData &k, int n) {                           \
    auto p = make_shared<Cls_##func>();                                        \
    p->setParam<int>("n", n);                                                  \
    p->setContext(k);                                                          \
    return Indicator(p);                                                       \
  }

#define TA_OC_OUT1_N_IMP(func, func_lookback, period, period_min, period_max) \
  Cls_##func::Cls_##func() : IndicatorImp(#func, 1) {                         \
    m_need_context = true;                                                    \
    setParam<int>("n", period);                                               \
  }                                                                           \
                                                                              \
  void Cls_##func::_checkParam(const string &name) const {                    \
    if (name == "n") {                                                        \
      int n = getParam<int>("n");                                             \
      HAYAKU_ASSERT(n >= period_min && n <= period_max);                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  void Cls_##func::_calculate(const Indicator &data) {                        \
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),                                \
                   "The input is ignored because {} depends on the context!", \
                   m_name);                                                   \
                                                                              \
    KData k = getContext();                                                   \
    size_t total = k.size();                                                  \
    HAYAKU_IF_RETURN(total == 0, void());                                     \
                                                                              \
    _readyBuffer(total, 1);                                                   \
    int n = getParam<int>("n");                                               \
    int back = func_lookback(n);                                              \
    if (back < 0 || back >= total) {                                          \
      m_discard = total;                                                      \
      return;                                                                 \
    }                                                                         \
                                                                              \
    const KRecord *kptr = k.data();                                           \
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);    \
    double *open = buf.get();                                                 \
    double *close = open + total;                                             \
    for (size_t i = 0; i < total; ++i) {                                      \
      open[i] = kptr[i].openPrice;                                            \
      close[i] = kptr[i].closePrice;                                          \
    }                                                                         \
                                                                              \
    auto *dst = this->data();                                                 \
    m_discard = back;                                                         \
    int outBegIdx;                                                            \
    int outNbElement;                                                         \
    ::func(m_discard, total - 1, open, close, n, &outBegIdx, &outNbElement,   \
           dst + m_discard);                                                  \
    HAYAKU_ASSERT((outBegIdx == m_discard) &&                                 \
                  (outBegIdx + outNbElement) <= total);                       \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(int n) {                                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    p->calculate();                                                           \
    return Indicator(p);                                                      \
  }                                                                           \
                                                                              \
  Indicator HAYAKU_API func(const KData &k, int n) {                          \
    auto p = make_shared<Cls_##func>();                                       \
    p->setParam<int>("n", n);                                                 \
    p->setContext(k);                                                         \
    return Indicator(p);                                                      \
  }
