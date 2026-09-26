#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 */


#if defined(_WIN32)
#if defined(HAYAKU_INGEST_BUILD)
#define HAYAKU_INGEST_API __declspec(dllexport)
#else
#define HAYAKU_INGEST_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define HAYAKU_INGEST_API __attribute__((visibility("default")))
#else
#define HAYAKU_INGEST_API
#endif
