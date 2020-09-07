/**
 * @file	intl_utils.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/08/31
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <codecvt>
#include <locale>

#include <string.h>

#include "intl_utils.h"

#include "simple_memory_pool.h"

namespace jcu {
namespace jvm {
namespace intl {

#ifdef _UNICODE
std::basic_string<system_char_t> utf8ToSystem(const char* text, int length) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>,wchar_t> s_utf8_to_wstr_convert;
  if (!text) {
    return std::basic_string<system_char_t>();
  }
  if (length < 0) {
    return s_utf8_to_wstr_convert.from_bytes(text);
  } else {
    return s_utf8_to_wstr_convert.from_bytes(text, length);
  }
}
std::string systemToUtf8(const system_char_t* text, int length) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>,wchar_t> s_utf8_to_wstr_convert;
  if (!text) {
    return std::string();
  }
  if (length < 0) {
    return s_utf8_to_wstr_convert.to_bytes(text);
  } else {
    return s_utf8_to_wstr_convert.to_bytes(text, length);
  }
}
#else
std::basic_string<system_char_t> utf8ToSystem(const char* text, int length) {
  if (!text) {
    return std::basic_string<system_char_t>();
  }
  if (length < 0) {
    return std::basic_string<system_char_t>(text);
  } else {
    return std::basic_string<system_char_t>(text, length);
  }
}

std::string systemToUtf8(const system_char_t* text, int length) {
  if (!text) {
    return std::string();
  }
  if (length < 0) {
    return std::string(text);
  } else {
    return std::string(text, length);
  }
}
#endif

char* mpollStrdup(MemoryPool* pool, const char* text) {
  size_t length = strlen(text);
  char* buf = (char*)pool->allocate(length + 1);
  memcpy(buf, text, length);
  buf[length] = 0;
  return buf;
}

} // namespace intl
} // namespace jvm
} // namespace jcu
