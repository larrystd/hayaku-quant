#pragma once

/**
 *  Copyright (c) 2021 hikyuu.org
 *
 *  Created on: 2021/05/19
 *      Author: fasiondog
 */

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/*
 * Macro definitions of the CPU architecture
 */
#if defined(__arm__) || defined(_M_ARM)
#define HAYAKU_ARCH_ARM 1
#define HAYAKU_ARCH_ARM64 0
#define HAYAKU_ARCH_X86 0
#define HAYAKU_ARCH_X64 0
#elif defined(__aarch64__) || defined(_M_ARM64)
#define HAYAKU_ARCH_ARM 0
#define HAYAKU_ARCH_ARM64 1
#define HAYAKU_ARCH_X86 0
#define HAYAKU_ARCH_X64 0
#elif defined(__x86_64__) || defined(_M_X64) || defined(_WIN64)
#define HAYAKU_ARCH_ARM 0
#define HAYAKU_ARCH_ARM64 0
#define HAYAKU_ARCH_X86 0
#define HAYAKU_ARCH_X64 1
#elif defined(__i386__) || defined(_M_IX86) || defined(_WIN32)
#define HAYAKU_ARCH_ARM 0
#define HAYAKU_ARCH_ARM64 0
#define HAYAKU_ARCH_X86 1
#define HAYAKU_ARCH_X64 0
#else
#define HAYAKU_ARCH_ARM 0
#define HAYAKU_ARCH_ARM64 0
#define HAYAKU_ARCH_X86 0
#define HAYAKU_ARCH_X64 0
#endif

/*
 * Macro definitions of the operating system
 */
#if defined(_WIN32) || defined(_WIN64)
#define HAYAKU_OS_WINDOWS 1
#define HAYAKU_OS_LINUX 0
#define HAYAKU_OS_ANDROID 0
#define HAYAKU_OS_OSX 0
#define HAYAKU_OS_IOS 0
#elif defined(__ANDROID__)
#define HAYAKU_OS_WINDOWS 0
#define HAYAKU_OS_LINUX 0
#define HAYAKU_OS_ANDROID 1
#define HAYAKU_OS_OSX 0
#define HAYAKU_OS_IOS 0
#elif defined(__linux__)
#define HAYAKU_OS_WINDOWS 0
#define HAYAKU_OS_LINUX 1
#define HAYAKU_OS_ANDROID 0
#define HAYAKU_OS_OSX 0
#define HAYAKU_OS_IOS 0
#elif TARGET_OS_OSX
#define HAYAKU_OS_WINDOWS 0
#define HAYAKU_OS_LINUX 0
#define HAYAKU_OS_ANDROID 0
#define HAYAKU_OS_OSX 1
#define HAYAKU_OS_IOS 0
#elif TARGET_OS_IOS || TARGET_OS_IPHONE
#define HAYAKU_OS_WINDOWS 0
#define HAYAKU_OS_LINUX 0
#define HAYAKU_OS_ANDROID 0
#define HAYAKU_OS_OSX 0
#define HAYAKU_OS_IOS 1
#else
#define HAYAKU_OS_WINDOWS 0
#define HAYAKU_OS_LINUX 0
#define HAYAKU_OS_ANDROID 0
#define HAYAKU_OS_OSX 0
#define HAYAKU_OS_IOS 0
#endif

// IOS simulator
#if HAYAKU_OS_IOS && TARGET_OS_SIMULATOR
#define HAYAKU_OS_IOS_SIMULATOR 1
#else
#define HAYAKU_OS_IOS_SIMULATOR 0
#endif
