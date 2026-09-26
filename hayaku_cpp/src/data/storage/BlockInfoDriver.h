#pragma once

/*
 * BlockInfoDriver.h
 *
 *  Created on: 2015-2-10
 *      Author: fasiondog
 */


#include "common/Parameter.h"
#include "data/Block.h"

namespace hayaku {

/**
 * Block data driver
 * @ingroup DataDriver
 */
class HAYAKU_API BlockInfoDriver {
    PARAMETER_SUPPORT

public:
    BlockInfoDriver(const string& name);
    virtual ~BlockInfoDriver() {};

    /** Get the driver name */
    const string& name() const;

    /**
     * Driver initialization
     * @param params
     * @return
     */
    bool init(const Parameter& params);

    /**
     * If a subclass needs a cache, it can implement this method to load the data into its own cache
     */
    virtual void load() {}

    /**
     * Driver initialization; when it is implemented concretely, attention should be paid to closing
     * the related resources opened before.
     */
    virtual bool _init() = 0;

    /**
     * Get all the block categories
     * @return StringList
     */
    virtual StringList getAllCategory() = 0;

    /**
     * Get the given block
     * @param category the given block category
     * @param name block name
     * @return the given block
     */
    virtual Block getBlock(const string& category, const string& name) = 0;

    /**
     * Get the block list of the given category
     * @param category block category
     * @return block list
     */
    virtual BlockList getBlockList(const string& category) = 0;

    /**
     * Get all the blocks
     * @return all the block lists
     */
    virtual BlockList getBlockList() = 0;

    /**
     * Save the given block
     * @note A block with the same name is overwritten; if the block category or name is modified,
     *       the original block needs to be deleted manually before the modification
     * @param block
     */
    virtual void save(const Block& block) = 0;

    /**
     * Delete the given block
     * @param category block category
     * @param name block name
     */
    virtual void remove(const string& category, const string& name) = 0;

protected:
    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

private:
    bool checkType();

protected:
    string m_name;
    bool m_is_python_object{false};
};

typedef shared_ptr<BlockInfoDriver> BlockInfoDriverPtr;

HAYAKU_API std::ostream& operator<<(std::ostream&, const BlockInfoDriver&);
HAYAKU_API std::ostream& operator<<(std::ostream&, const BlockInfoDriverPtr&);

inline const string& BlockInfoDriver::name() const {
    return m_name;
}

} /* namespace hayaku */
