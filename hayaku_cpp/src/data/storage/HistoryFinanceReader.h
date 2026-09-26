#pragma once

/*
 * HistoryFinanceReader.h
 *
 * Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */


#include "data/Stock.h"

namespace hayaku {

/**
 * Read the historical financial information
 * @ingroup DataDriver
 */
class HAYAKU_API HistoryFinanceReader {
public:
    HistoryFinanceReader() = delete;
    explicit HistoryFinanceReader(const string& dir);
    virtual ~HistoryFinanceReader();

    PriceList getHistoryFinanceInfo(Datetime date, const string& market, const string& code);

private:
    string m_dir;  // The directory where the historical financial information files are stored
};

}  // namespace hayaku
