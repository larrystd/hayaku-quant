#pragma once

/*
 * QLBlockInfoDriver.h
 *
 *  Created on: 2015-2-10
 *      Author: fasiondog
 */


#include "data/storage/BlockInfoDriver.h"

namespace hayaku {

class QLBlockInfoDriver : public BlockInfoDriver {
public:
    QLBlockInfoDriver() : BlockInfoDriver("qianlong") {};
    virtual ~QLBlockInfoDriver() override;

    virtual bool _init() override;
    virtual StringList getAllCategory() override;
    virtual Block getBlock(const string&, const string&) override;
    virtual BlockList getBlockList(const string& category) override;
    virtual BlockList getBlockList() override;
    virtual void save(const Block& block) override;
    virtual void remove(const string& category, const string& name) override;
};

} /* namespace hayaku */
