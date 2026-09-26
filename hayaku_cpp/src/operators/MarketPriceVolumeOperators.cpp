#include "MarketOperators.h"

/*
 * IAd.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */


#include "Indicator.h"

namespace hayaku {

class IAd : public IndicatorImp {
    INDICATOR_IMP(IAd)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAd();
    virtual ~IAd() override;
};

} /* namespace hayaku */

/*
 * IAd.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAd)
#endif

namespace hayaku {

IAd::IAd() : IndicatorImp("AD", 1) {
    m_need_context = true;
}

IAd::~IAd() {}

void IAd::_calculate(const Indicator& data) {
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    m_discard = 0;
    const KData& k = getContext();
    size_t total = k.size();
    HAYAKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    value_t ad = 0.0;
    auto* dst = this->data();
    dst[m_discard] = 0.0;
    for (size_t i = m_discard + 1; i < total; i++) {
        const KRecord& r = k[i];
        value_t tmp = r.highPrice - r.lowPrice;
        if (tmp != 0.0) {
            // Bull/bear comparison = [(close - low) - (high - close)] / (high - low)
            ad += ((r.closePrice + r.closePrice - r.highPrice - r.lowPrice) / tmp) * r.transAmount;
        }
        dst[i] = ad;
    }
}

Indicator HAYAKU_API AD() {
    return make_shared<IAd>()->calculate();
}

Indicator HAYAKU_API AD(const KData& k) {
    auto p = make_shared<IAd>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-22
 *      Author: fasiondog
 */



namespace hayaku {

/* COST(k, X) means what the price is when X% of the positions are profitable */
class ICost : public IndicatorImp {
    INDICATOR_IMP(ICost)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICost();
    virtual ~ICost() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICost)
#endif

namespace hayaku {

ICost::ICost() : IndicatorImp("COST", 1) {
    m_need_context = true;
    setParam<double>("percent", 10.0);
}

ICost::~ICost() {}

void ICost::_checkParam(const string& name) const {
    if (name == "percent") {
        double percent = getParam<double>("percent");
        HAYAKU_CHECK(percent >= 0.0 && percent <= 100.0, "Invalid param percent: {}", percent);
    }
}

// Assume the cost distribution: DMA(x, HSL=A) = A*X+(1-A)*Y'
// The actual algorithm: DMA(CLOSE() + (HIGH() - LOW()) * x / 100.0, HSL());
void ICost::_calculate(const Indicator& data) {
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HAYAKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    // Set the discard to everything first, it is updated later
    m_discard = total;

    Stock stock = k.getStock();
    auto* kdata = k.data();
    Datetime lastdate = kdata[total - 1].datetime.startOfDay();

    StockWeightList sw_list = stock.getWeight(Datetime::min(), lastdate + Days(1));
    HAYAKU_IF_RETURN(sw_list.empty(), void());

    // Find the first ex-rights/ex-dividend record whose outstanding shares are not 0
    price_t pre_free_count = 0.0;
    Datetime pre_sw_date;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        if (sw_iter->freeCount() > 0) {
            pre_free_count = sw_iter->freeCount();
            pre_sw_date = sw_iter->datetime();
            break;
        }
    }

    // Return directly when there is no ex-rights/ex-dividend data with outstanding shares, or when
    // the date of that record is later than the last K-line date
    HAYAKU_IF_RETURN(sw_iter == sw_list.end() || pre_sw_date > lastdate, void());

    auto* dst = this->data();

    value_t percent = getParam<double>("percent") * 0.01;
    size_t pos = 0;
    value_t x, a;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t free_count = sw_iter->freeCount();
        Datetime cur_sw_date = sw_iter->datetime();
        if (free_count <= 0.0) {
            continue;  // Ignore the ex-rights/ex-dividend record whose outstanding shares are 0
        }

        while (pos < total && kdata[pos].datetime < cur_sw_date) {
            const KRecord& krecord = kdata[pos];
            if (krecord.datetime >= pre_sw_date) {
                x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
                // transCount is in lots and the outstanding shares are in units of 10 thousand
                // shares
                a = krecord.transCount / pre_free_count * 0.01;
                dst[pos] = pos > 0 ? a * x + (1 - a) * dst[pos - 1] : x;
            }
            pos++;
        }

        pre_free_count = free_count;
        pre_sw_date = cur_sw_date;
        if (pos >= total) {
            break;
        }
    }

    if (pos == 0) {
        const KRecord& krecord = kdata[pos];
        x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
        dst[pos] = x;
        pos++;
    }

    for (; pos < total; pos++) {
        const KRecord& krecord = kdata[pos];
        x = krecord.closePrice + (krecord.highPrice - krecord.lowPrice) * percent;
        a = krecord.transCount / pre_free_count * 0.01;
        dst[pos] = a * x + (1 - a) * dst[pos - 1];
    }

    // Update the discard
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
    }
}

Indicator HAYAKU_API COST(double x) {
    auto p = make_shared<ICost>();
    p->setParam<double>("percent", x);
    return Indicator(p);
}

Indicator HAYAKU_API COST(const KData& k, double x) {
    auto p = make_shared<ICost>();
    p->setParam<double>("percent", x);
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-22
 *      Author: fasiondog
 */



namespace hayaku {

/* Get the turnover rate, it equals VOL(k) / CAPITAL(k) */
class IHsl : public IndicatorImp {
    INDICATOR_IMP(IHsl)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IHsl();
    virtual ~IHsl() override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-22
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IHsl)
#endif

namespace hayaku {

IHsl::IHsl() : IndicatorImp("HSL", 1) {
    m_need_context = true;
}

IHsl::~IHsl() {}

void IHsl::_calculate(const Indicator& data) {
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HAYAKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    // Set the discard to everything first, it is updated later
    m_discard = total;

    Stock stock = k.getStock();
    auto* kdata = k.data();
    Datetime lastdate = kdata[total - 1].datetime.startOfDay();

    StockWeightList sw_list = stock.getWeight(Datetime::min(), lastdate + Days(1));
    HAYAKU_IF_RETURN(sw_list.empty(), void());

    // Find the first ex-rights/ex-dividend record whose outstanding shares are not 0
    price_t pre_free_count = 0.0;
    Datetime pre_sw_date;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        if (sw_iter->freeCount() > 0) {
            pre_free_count = sw_iter->freeCount();
            pre_sw_date = sw_iter->datetime();
            break;
        }
    }

    // Return directly when there is no ex-rights/ex-dividend data with outstanding shares, or when
    // the date of that record is later than the last K-line date
    HAYAKU_IF_RETURN(sw_iter == sw_list.end() || pre_sw_date > lastdate, void());

    auto* dst = this->data();
    size_t pos = 0;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t free_count = sw_iter->freeCount();
        Datetime cur_sw_date = sw_iter->datetime();
        if (free_count <= 0.0) {
            continue;  // Ignore the ex-rights/ex-dividend record whose outstanding shares are 0
        }

        while (pos < total && kdata[pos].datetime < cur_sw_date) {
            if (kdata[pos].datetime >= pre_sw_date) {
                // transCount is in lots and the outstanding shares are in units of 10 thousand
                // shares
                dst[pos] = kdata[pos].transCount / pre_free_count * 0.01;
            }
            pos++;
        }

        pre_free_count = free_count;
        pre_sw_date = cur_sw_date;
        if (pos >= total) {
            break;
        }
    }

    for (; pos < total; pos++) {
        dst[pos] = kdata[pos].transCount / pre_free_count * 0.01;
    }

    // Update the discard
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
    }
}

Indicator HAYAKU_API HSL() {
    return make_shared<IHsl>()->calculate();
}

Indicator HAYAKU_API HSL(const KData& k) {
    auto p = make_shared<IHsl>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-09
 *      Author: fasiondog
 */



namespace hayaku {

class IIndex : public IndicatorImp {
    INDICATOR_IMP(IIndex)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIndex();
    IIndex(const string& kpart, bool fill_null);
    virtual ~IIndex() override;

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "data/StockTypeInfo.h"
#include "SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIndex)
#endif

namespace hayaku {

IIndex::IIndex() : IndicatorImp("INDEX", 1) {
    setParam<bool>("fill_null", true);
    setParam<string>("kpart", "CLOSE");
    setParam<string>("market_code", "SH000001");
}

IIndex::IIndex(const string& kpart, bool fill_null) : IndicatorImp("INDEX", 1) {
    setParam<bool>("fill_null", fill_null);
    string part_name(kpart);
    to_upper(part_name);
    setParam<string>("kpart", part_name);
    setParam<string>("market_code", "SH000001");
}

IIndex::~IIndex() {}

void IIndex::_checkParam(const string& name) const {
    if ("kpart" == name) {
        string part = getParam<string>("kpart");
        HAYAKU_ASSERT("OPEN" == part || "HIGH" == part || "LOW" == part || "CLOSE" == part ||
                   "AMO" == part || "VOL" == part);
    }
}

void IIndex::_calculate(const Indicator& ind) {
    const auto& k = getContext();
    size_t total = k.size();
    HAYAKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    string market_code = getParam<string>("market_code");
    Stock stk = k.getStock();
    if (stk.type() == STOCKTYPE_A) {
        if (stk.market() == "SH") {
            market_code = "SH000001";
        } else if (stk.market() == "SZ") {
            market_code = "SZ399001";
        } else {
            HAYAKU_WARN("Not known the index code, will use SH000001 as default.");
        }
    } else if (stk.type() == STOCKTYPE_A_BJ) {
        market_code = "BJ899050";
    } else if (stk.type() == STOCKTYPE_START) {
        market_code = "SH000688";
    } else if (stk.type() == STOCKTYPE_GEM) {
        market_code = "SZ399006";
    } else {
        HAYAKU_WARN("Not known the index code, will use {} as default.", market_code);
    }

    // Adjust the market_code parameter to the market_code of the current index
    setParam<string>("market_code", market_code);

    KQuery query = k.getQuery();
    auto secs = KQuery::getKTypeInSeconds(query.kType());
    query = KQueryByDate(k[0].datetime, k[total - 1].datetime + Seconds(secs), query.kType(),
                         query.recoverType());

    KData index_k = getKData(market_code, query);
    Indicator index =
      ALIGN(KDATA_PART(index_k, getParam<string>("kpart")), k, getParam<bool>("fill_null"));

    HAYAKU_ASSERT(index.size() == total);
    const auto* src = index.data();
    auto* dst = this->data();
    for (size_t i = 0; i < total; i++) {
        dst[i] = src[i];
    }
}

static Indicator INDEX(const string& kpart, bool fill_null) {
    IndicatorImpPtr p = make_shared<IIndex>(kpart, fill_null);
    if ("OPEN" == kpart) {
        p->name("INDEXO");
    } else if ("HIGH" == kpart) {
        p->name("INDEXH");
    } else if ("LOW" == kpart) {
        p->name("INDEXL");
    } else if ("CLOSE" == kpart) {
        p->name("INDEXC");
    } else if ("AMO" == kpart) {
        p->name("INDEXA");
    } else if ("VOL" == kpart) {
        p->name("INDEXV");
    }
    return p->calculate();
}

Indicator HAYAKU_API INDEXO(bool fill_null) {
    return INDEX("OPEN", fill_null);
}

Indicator HAYAKU_API INDEXH(bool fill_null) {
    return INDEX("HIGH", fill_null);
}

Indicator HAYAKU_API INDEXL(bool fill_null) {
    return INDEX("LOW", fill_null);
}

Indicator HAYAKU_API INDEXC(bool fill_null) {
    return INDEX("CLOSE", fill_null);
}

Indicator HAYAKU_API INDEXA(bool fill_null) {
    return INDEX("AMO", fill_null);
}

Indicator HAYAKU_API INDEXV(bool fill_null) {
    return INDEX("VOL", fill_null);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-16
 *      Author: fasiondog
 */


namespace hayaku {

Indicator HAYAKU_API INDEXADV() {
    KData k = getKData("SH880005", KQueryByIndex(-1));
    return CONTEXT(k.close());
}

Indicator HAYAKU_API INDEXADV(const KQuery& query) {
    KData k = getKData("SH880005", query);
    return CONTEXT(k.close());
}

Indicator HAYAKU_API INDEXDEC() {
    KData k = getKData("SH880005", KQueryByIndex(-1));
    return CONTEXT(k.open());
}

Indicator HAYAKU_API INDEXDEC(const KQuery& query) {
    KData k = getKData("SH880005", query);
    return CONTEXT(k.open());
}

}  // namespace hayaku

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */



namespace hayaku {

/*
 * Implementation of the TDX time functions
 */
class ITime : public IndicatorImp {
    INDICATOR_IMP(ITime)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITime();
    virtual ~ITime() override;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ITime)
#endif

namespace hayaku {

ITime::ITime() : IndicatorImp("TIME") {
    m_need_context = true;
    setParam<string>("type", "TIME");
}

ITime::~ITime() {}

void ITime::_checkParam(const string& name) const {
    if ("type" == name) {
        string param_type = getParam<string>("type");
        HAYAKU_CHECK("TIME" == param_type || "DATE" == param_type || "YEAR" == param_type ||
                    "MONTH" == param_type || "WEEK" == param_type || "DAY" == param_type ||
                    "HOUR" == param_type || "MINUTE" == param_type,
                  "Invalid param type: {}", param_type);
    }
}

void ITime::_calculate(const Indicator& data) {
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& kdata = getContext();
    size_t total = kdata.size();
    HAYAKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    _increment_calculate(data, 0);
}

void ITime::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& kdata = getContext();
    size_t total = kdata.size();
    DatetimeList ds = kdata.getDatetimeList();

    auto* dst = this->data();
    string type_name = getParam<string>("type");
    if ("TIME" == type_name) {
        for (size_t i = start_pos; i < total; i++) {
            const auto& d = ds[i];
            dst[i] = d.hour() * 10000 + d.minute() * 100 + d.second();
        }

    } else if ("DATE" == type_name) {
        for (size_t i = start_pos; i < total; i++) {
            const auto& d = ds[i];
            dst[i] = (d.year() - 1900) * 10000 + d.month() * 100 + d.day();
        }

    } else if ("YEAR" == type_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = ds[i].year();
        }

    } else if ("MONTH" == type_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = ds[i].month();
        }

    } else if ("WEEK" == type_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = ds[i].dayOfWeek();
        }

    } else if ("DAY" == type_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = ds[i].day();
        }

    } else if ("HOUR" == type_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = ds[i].hour();
        }

    } else if ("MINUTE" == type_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = ds[i].minute();
        }
    }
}

Indicator HAYAKU_API DATE(const KData& kdata) {
    auto p = make_shared<ITime>();
    p->setParam<string>("type", "DATE");
    p->name("DATE");
    p->setContext(kdata);
    return Indicator(p);
}

Indicator HAYAKU_API TIME(const KData& kdata) {
    auto p = make_shared<ITime>();
    p->setParam<string>("type", "TIME");
    p->name("TIME");
    p->setContext(kdata);
    return Indicator(p);
}

Indicator HAYAKU_API YEAR(const KData& kdata) {
    auto p = make_shared<ITime>();
    p->setParam<string>("type", "YEAR");
    p->name("YEAR");
    p->setContext(kdata);
    return Indicator(p);
}

Indicator HAYAKU_API MONTH(const KData& kdata) {
    auto p = make_shared<ITime>();
    p->setParam<string>("type", "MONTH");
    p->name("MONTH");
    p->setContext(kdata);
    return Indicator(p);
}

Indicator HAYAKU_API WEEK(const KData& kdata) {
    auto p = make_shared<ITime>();
    p->setParam<string>("type", "WEEK");
    p->name("WEEK");
    p->setContext(kdata);
    return Indicator(p);
}

Indicator HAYAKU_API DAY(const KData& kdata) {
    auto p = make_shared<ITime>();
    p->setParam<string>("type", "DAY");
    p->name("DAY");
    p->setContext(kdata);
    return Indicator(p);
}

Indicator HAYAKU_API HOUR(const KData& kdata) {
    auto p = make_shared<ITime>();
    p->setParam<string>("type", "HOUR");
    p->name("HOUR");
    p->setContext(kdata);
    return Indicator(p);
}

Indicator HAYAKU_API MINUTE(const KData& kdata) {
    auto p = make_shared<ITime>();
    p->setParam<string>("type", "MINUTE");
    p->name("MINUTE");
    p->setContext(kdata);
    return Indicator(p);
}

//-----------------------------------------------------------
Indicator HAYAKU_API DATE() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "DATE");
    p->name("DATE");
    return p->calculate();
}

Indicator HAYAKU_API TIME() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "TIME");
    p->name("TIME");
    return p->calculate();
}

Indicator HAYAKU_API YEAR() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "YEAR");
    p->name("YEAR");
    return p->calculate();
}

Indicator HAYAKU_API MONTH() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "MONTH");
    p->name("MONTH");
    return p->calculate();
}

Indicator HAYAKU_API WEEK() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "WEEK");
    p->name("WEEK");
    return p->calculate();
}

Indicator HAYAKU_API DAY() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "DAY");
    p->name("DAY");
    return p->calculate();
}

Indicator HAYAKU_API HOUR() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "HOUR");
    p->name("HOUR");
    return p->calculate();
}

Indicator HAYAKU_API MINUTE() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "MINUTE");
    p->name("MINUTE");
    return p->calculate();
}

}  // namespace hayaku

/*
 * ITimeLine.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */



namespace hayaku {

class ITimeLine : public IndicatorImp {
    INDICATOR_IMP(ITimeLine)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITimeLine();
    virtual ~ITimeLine() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

/*
 * ITimeLine.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ITimeLine)
#endif

namespace hayaku {

ITimeLine::ITimeLine() : IndicatorImp("TIMELINE", 1) {
    m_need_context = true;
    setParam<string>("part", "price");
}

ITimeLine::~ITimeLine() {}

void ITimeLine::_checkParam(const string& name) const {
    if ("part" == name) {
        string part = getParam<string>("part");
        HAYAKU_ASSERT(part == "price" || part == "vol");
    }
}

void ITimeLine::_calculate(const Indicator& data) {
    HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    KQuery q = k.getQuery();
    Stock stk = k.getStock();

    TimeLineList time_line = stk.getTimeLineList(q);
    size_t total = time_line.size();
    HAYAKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    auto* dst = this->data();

    m_discard = 0;
    if (getParam<string>("part") == "price") {
        for (size_t i = m_discard; i < total; i++) {
            dst[i] = time_line[i].price;
        }
    } else {
        for (size_t i = m_discard; i < total; i++) {
            dst[i] = time_line[i].vol;
        }
    }
}

Indicator HAYAKU_API TIMELINE() {
    return make_shared<ITimeLine>()->calculate();
}

Indicator HAYAKU_API TIMELINE(const KData& k) {
    auto p = make_shared<ITimeLine>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */


namespace hayaku {

Indicator HAYAKU_API TIMELINEVOL() {
    Indicator ind = TIMELINE();
    ind.name("TIMELINEVOL");
    ind.setParam<string>("part", "vol");
    return ind;
}

Indicator HAYAKU_API TIMELINEVOL(const KData& k) {
    Indicator ind = TIMELINE(k);
    ind.name("TIMELINEVOL");
    ind.setParam<string>("part", "vol");
    return ind;
}

}

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-06
 *      Author: fasiondog
 */

#include "WindowOperators.h"

namespace hayaku {

// No need to multiply by 100, the trading volume is already in lots, i.e. 100
Indicator HAYAKU_API TURNOVER(int n) {
    HAYAKU_ASSERT(n >= 1);
    return n == 1 ? (VOL() / LIUTONGPAN()) : (SUM(VOL(), n) / SUM(LIUTONGPAN(), n));
}

Indicator HAYAKU_API TURNOVER(const KData& kdata, int n) {
    HAYAKU_ASSERT(n >= 1);
    return n == 1 ? (kdata.vol() / LIUTONGPAN(kdata))
                  : (SUM(kdata.vol(), n) / SUM(LIUTONGPAN(kdata), n));
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */



namespace hayaku {

class IWinner : public IndicatorImp {
    INDICATOR_IMP(IWinner)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IWinner();
    virtual ~IWinner() override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */

#include "common/concurrency/ParallelAlgorithms.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IWinner)
#endif

namespace hayaku {

IWinner::IWinner() : IndicatorImp("WINNER", 1) {}

IWinner::~IWinner() {}

void IWinner::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    // Get the context of the input indicator
    auto context = data.getContext();
    if (context == Null<KData>()) {
        m_discard = total;
        return;
    }

    IndicatorList cost_list(101);
    value_t const *cost_data[101];

    cost_list[0] = COST(0)(context);
    m_discard = cost_list[0].discard();
    HAYAKU_IF_RETURN(m_discard >= total, void());
    cost_data[0] = cost_list[0].data();

    global_parallel_for_index_void(1, 101, [&cost_data, &cost_list, &context](size_t i) {
        cost_list[i] = COST(i)(context);
        cost_data[i] = cost_list[i].data();
    });

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        int high_idx = 100;
        int low_idx = 0;
        while (low_idx <= high_idx) {
            value_t high = cost_data[high_idx][i];
            value_t low = cost_data[low_idx][i];
            int mid_idx = (high_idx + low_idx) / 2;
            value_t mid = cost_data[mid_idx][i];
            if (src[i] >= high) {
                dst[i] = high_idx * 0.01;
                break;
            } else if (src[i] <= low) {
                dst[i] = low_idx * 0.01;
                break;
            } else if (src[i] == mid) {
                dst[i] = mid_idx * 0.01;
                break;
            }

            if (src[i] > mid) {
                low_idx = mid_idx + 1;
                if (low_idx >= high_idx) {
                    dst[i] = low_idx * 0.01;
                    break;
                }
            } else {
                high_idx = mid_idx - 1;
                if (high_idx <= low_idx) {
                    dst[i] = low_idx * 0.01;
                    break;
                }
            }
        }
    }
}

Indicator HAYAKU_API WINNER() {
    return Indicator(make_shared<IWinner>());
}

} /* namespace hayaku */
