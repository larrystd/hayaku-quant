#include "SeriesOperators.h"

// ---- Merged implementation type from IRecover.h ----
/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240317 added by fasiondog
 */
#include "Indicator.h"

namespace hayaku {

class IRecover : public IndicatorImp {
    INDICATOR_IMP(IRecover)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRecover();
    explicit IRecover(int recoverType);
    IRecover(const KData&, int recoverType);
    virtual ~IRecover() override;

    virtual void _checkParam(const string& name) const override;

    static void checkInputIndicator(const Indicator& ind);

    // virtual bool supportIncrementCalculate() const override;
    // virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

}  // namespace hayaku

// ---- Merged implementation type from IRef.h ----
/*
 * IRef.h
 *
 *  Created on: 2015-3-21
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * REF forward reference (i.e. shift right)
 * Reference the data of several periods before.
 * Usage: REF(X,A) references the X value A periods before.
 * For example: REF(CLOSE,1) means the close price of the previous period, which is the previous
 * close on the daily line.
 */
class IRef : public IndicatorImp {
    INDICATOR_IMP(IRef)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRef();
    virtual ~IRef() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */


// ---- Merged implementation type from IRefX.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-22
 *      Author: fasiondog
 */

namespace hayaku {

class IRefX : public IndicatorImp {
    INDICATOR_IMP(IRefX)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRefX();
    virtual ~IRefX() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ILastValue.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-04
 *      Author: fasiondog
 */

namespace hayaku {

class ILastValue : public IndicatorImp {
    INDICATOR_IMP(ILastValue)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILastValue();
    virtual ~ILastValue() override;
};

} /* namespace hayaku */

// ---- Merged implementation from IRecover.cpp ----
/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240317 added by fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRecover)
#endif

namespace hayaku {

IRecover::IRecover() : IndicatorImp("RECOVER") {
    setParam<int>("recover_type", KQuery::NO_RECOVER);
}

IRecover::IRecover(int recoverType) {
    setParam<int>("recover_type", recoverType);
}

IRecover::IRecover(const KData& kdata, int recoverType) : IndicatorImp("RECOVER") {
    setParam<int>("recover_type", recoverType);
    onlySetContext(kdata);
}

IRecover::~IRecover() {}

void IRecover::_checkParam(const string& name) const {
    if ("recover_type" == name) {
        int recover_type = getParam<int>("recover_type");
        HAYAKU_ASSERT(recover_type >= KQuery::NO_RECOVER &&
                   recover_type < KQuery::INVALID_RECOVER_TYPE);
    }
}

void IRecover::checkInputIndicator(const Indicator& ind) {
    HAYAKU_CHECK(dynamic_cast<IKData*>(ind.getImp().get()) != nullptr,
              "Only the following indicators are accepted: OPEN|HIGH|CLOSE|LOW");
    string part = ind.getParam<string>("kpart");
    HAYAKU_CHECK(part == "CLOSE" || part == "OPEN" || part == "HIGH" || part == "LOW" ||
                part == "AMO" || part == "VOL",
              "Only the following indicators are accepted: OPEN|HIGH|CLOSE|LOW");
}

void IRecover::_calculate(const Indicator& ind) {
    auto kdata = ind.getContext();
    auto query = kdata.getQuery();

    KQuery::RecoverType recover_type =
      static_cast<KQuery::RecoverType>(getParam<int>("recover_type"));
    m_name = fmt::format("RECOVER_{}", KQuery::getRecoverTypeName(recover_type));

    query.recoverType(recover_type);
    KData new_k = kdata.getKData(query);
    HAYAKU_ASSERT(new_k.size() == ind.size());

    size_t total = new_k.size();
    _readyBuffer(total, 1);

    string part_name = ind.getParam<string>("kpart");
    const auto* data = new_k.data();
    auto* dst = this->data();
    if ("CLOSE" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].closePrice;
        }

    } else if ("OPEN" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].openPrice;
        }

    } else if ("HIGH" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].highPrice;
        }

    } else if ("LOW" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].lowPrice;
        }

    } else if ("AMO" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].transAmount;
        }

    } else if ("VOL" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].transCount;
        }
    }
}

#if 0
// It is meaningful only when the back adjustment is in the full mode, but the full back adjustment is too slow
bool IRecover::supportIncrementCalculate() const {
    KQuery::RecoverType recover_type =
      static_cast<KQuery::RecoverType>(getParam<int>("recover_type"));
    return !(recover_type == KQuery::FORWARD || recover_type == KQuery::EQUAL_FORWARD);
}

void IRecover::_increment_calculate(const Indicator& ind, size_t start_pos) {
    auto kdata = ind.getContext();
    auto query = kdata.getQuery();

    KQuery::RecoverType recover_type =
      static_cast<KQuery::RecoverType>(getParam<int>("recover_type"));

    // Guarantee that the data from the old context start to the new context end are all calculated
    query = KQueryByDate(m_old_context.front().datetime,
                         kdata.back().datetime + Seconds(KQuery::getKTypeInSeconds(query.kType())),
                         query.kType(), recover_type);
    KData new_k = m_old_context.getKData(query);

    size_t pos = new_k.getPos(kdata[start_pos].datetime);
    HAYAKU_ASSERT(new_k.size() == (pos + ind.size() - start_pos));

    size_t total = ind.size();

    string part_name = ind.getParam<string>("kpart");
    const auto* data = new_k.data();
    auto* dst = this->data();
    if ("CLOSE" == part_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = data[pos++].closePrice;
        }

    } else if ("OPEN" == part_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = data[pos++].openPrice;
        }

    } else if ("HIGH" == part_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = data[pos++].highPrice;
        }

    } else if ("LOW" == part_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = data[pos++].lowPrice;
        }
    } else if ("AMO" == part_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = data[pos++].transAmount;
        }

    } else if ("VOL" == part_name) {
        for (size_t i = start_pos; i < total; i++) {
            dst[i] = data[pos++].transCount;
        }
    }
}
#endif

Indicator HAYAKU_API RECOVER_FORWARD() {
    return Indicator(make_shared<IRecover>(KQuery::FORWARD));
}

Indicator HAYAKU_API RECOVER_BACKWARD() {
    return Indicator(make_shared<IRecover>(KQuery::BACKWARD));
}

Indicator HAYAKU_API RECOVER_EQUAL_FORWARD() {
    return Indicator(make_shared<IRecover>(KQuery::EQUAL_FORWARD));
}

Indicator HAYAKU_API RECOVER_EQUAL_BACKWARD() {
    return Indicator(make_shared<IRecover>(KQuery::EQUAL_BACKWARD));
}

Indicator HAYAKU_API RECOVER_FORWARD(const Indicator& ind) {
    IRecover::checkInputIndicator(ind);
    return RECOVER_FORWARD()(ind);
}

Indicator HAYAKU_API RECOVER_BACKWARD(const Indicator& ind) {
    IRecover::checkInputIndicator(ind);
    return RECOVER_BACKWARD()(ind);
}

Indicator HAYAKU_API RECOVER_EQUAL_FORWARD(const Indicator& ind) {
    IRecover::checkInputIndicator(ind);
    return RECOVER_EQUAL_FORWARD()(ind);
}

Indicator HAYAKU_API RECOVER_EQUAL_BACKWARD(const Indicator& ind) {
    IRecover::checkInputIndicator(ind);
    return RECOVER_EQUAL_BACKWARD()(ind);
}

}  // namespace hayaku

// ---- Merged implementation from IRef.cpp ----
/*
 * IRef.cpp
 *
 *  Created on: 2015-3-21
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRef)
#endif

namespace hayaku {

IRef::IRef() : IndicatorImp("REF", 1) {
    setParam<int>("n", 1);
}

IRef::~IRef() {}

void IRef::_checkParam(const string& name) const {
    if ("n" == name) {
        HAYAKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IRef::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IRef::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, total = data.size(); i < total; ++i) {
        dst[i] = src[i - n];
    }
}

void IRef::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (curPos >= step) {
        _set(ind[curPos - step], curPos);
    }
}

Indicator HAYAKU_API REF(int n) {
    IndicatorImpPtr p = make_shared<IRef>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HAYAKU_API REF(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IRef>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IRefX.cpp ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-22
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRefX)
#endif

namespace hayaku {

IRefX::IRefX() : IndicatorImp("REFX", 1) {
    setParam<int>("n", 1);
}

IRefX::~IRefX() {}

void IRefX::_calculate(const Indicator &data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    if (0 == n) {
        m_discard = data.discard();
        const auto *src = data.data() + m_discard;
        auto *dst = this->data() + m_discard;
        memcpy(dst, src, (total - m_discard) * sizeof(value_t));
        return;

    } else if (n > 0) {
        m_discard = data.discard() + n;
        if (m_discard >= total) {
            m_discard = total;
            return;
        }

        const auto *src = data.data() + data.discard();
        auto *dst = this->data() + m_discard;
        memcpy(dst, src, (total - m_discard) * sizeof(value_t));
        return;

    } else {
        size_t absn = std::abs(n);
        if (absn >= total) {
            m_discard = total;
            return;
        }

        int64_t startix = data.discard() - absn;
        size_t len = total - data.discard();
        if (startix < 0) {
            m_discard = 0;
            len = total - absn;
        } else {
            m_discard = startix;
        }

        const auto *src = data.data() + total - len;
        auto *dst = this->data() + m_discard;
        memcpy(dst, src, len * sizeof(value_t));
        return;
    }
}

Indicator HAYAKU_API REFX(int n) {
    IndicatorImpPtr p = make_shared<IRefX>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from ILastValue.cpp ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-04
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ILastValue)
#endif

namespace hayaku {

ILastValue::ILastValue() : IndicatorImp("LASTVALUE", 1) {
    setParam<bool>("ignore_discard", false);  // Ignore the discard of the input indicator
}

ILastValue::~ILastValue() {}

void ILastValue::_calculate(const Indicator &data) {
    size_t total = data.size();
    HAYAKU_IF_RETURN(total == 0, void());

    bool ignore_discard = getParam<bool>("ignore_discard");
    if (!ignore_discard) {
        m_discard = data.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
    }

    value_t last_val = data[total - 1];
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = last_val;
    }
}

Indicator HAYAKU_API LASTVALUE(bool ignore_discard) {
    auto p = make_shared<ILastValue>();
    p->setParam<bool>("ignore_discard", ignore_discard);
    return Indicator(p);
}

} /* namespace hayaku */
