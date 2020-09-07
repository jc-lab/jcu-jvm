/**
 * @file	intl_utils.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/08/31
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef COMMONS_DAEMON_NATIVE_SRC_JVM_HELPER_UTILS_H_
#define COMMONS_DAEMON_NATIVE_SRC_JVM_HELPER_UTILS_H_

#ifdef _MSC_VER
#include <tchar.h>
typedef TCHAR system_char_t;
#else
typedef char system_char_t;
#endif

#include <string>

#include <jcu-jvm/memory_pool.h>

namespace jcu {
namespace jvm {
namespace intl {

class SimpleMemoryPool;

std::basic_string<system_char_t> utf8ToSystem(const char* text, int length = -1);
std::string systemToUtf8(const system_char_t* text, int length = -1);

template <typename T>
std::basic_string<T> stringReplace(std::basic_string<T> input, const std::basic_string<T>& find, const std::basic_string<T>& rep) {
  std::string::size_type pos = 0u;
  while((pos = input.find(find, pos)) != std::string::npos){
    input.replace(pos, find.length(), rep);
    pos += rep.length();
  }
  return input;
}

char* mpollStrdup(MemoryPool* pool, const char* text);

} // namespace intl
} // namespace jvm
} // namespace jcu

#endif //COMMONS_DAEMON_NATIVE_SRC_JVM_HELPER_UTILS_H_
