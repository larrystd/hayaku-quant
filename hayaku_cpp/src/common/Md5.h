#pragma once

/*
 *  Copyright (c) 2019~2021, hayaku
 *
 *  Created on: 2021/12/06
 *      Author: fasiondog
 */

#include <string>

#ifndef HAYAKU_UTILS_API
#define HAYAKU_UTILS_API
#endif

namespace hayaku {

/**
 * @brief Calculate the md5 value
 *
 * @param input the start pointer of the data to be calculated
 * @param len the byte length of the data to be calculated
 * @return std::string
 */
std::string HAYAKU_UTILS_API md5(const unsigned char* input, size_t len);

/**
 * @brief Calculate the md5 of a string
 *
 * @param src the string to be calculated
 * @return std::string
 */
inline std::string md5(const std::string& src) {
  return md5((const unsigned char*)src.data(), src.size());
}

}  // namespace hayaku
