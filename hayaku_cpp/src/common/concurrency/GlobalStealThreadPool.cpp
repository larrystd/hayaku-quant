/**
 *  Copyright (c) 2021 hikyuu.org
 *
 *  Created on: 2021/05/19
 *      Author: fasiondog
 */

#include "GlobalStealThreadPool.h"

#include "common/OsDef.h"

namespace hayaku {

#if HAYAKU_OS_WINDOWS
WorkStealQueue* GlobalStealThreadPool::local_work_queue_ = nullptr;
int GlobalStealThreadPool::index_ = -1;
InterruptFlag GlobalStealThreadPool::thread_need_stop_;
std::thread::id GlobalStealThreadPool::thread_id_;

#else

#if CPP_STANDARD < CPP_STANDARD_17 || defined(__clang__)
thread_local WorkStealQueue* GlobalStealThreadPool::local_work_queue_ =
    nullptr;
thread_local int GlobalStealThreadPool::index_ = -1;
thread_local InterruptFlag GlobalStealThreadPool::thread_need_stop_;
thread_local std::thread::id GlobalStealThreadPool::thread_id_;
#endif

#endif

}  // namespace hayaku
