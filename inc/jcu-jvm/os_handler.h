/**
 * @file	os_handler.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/01
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef JCU_JVM_OS_HANDLER_H_
#define JCU_JVM_OS_HANDLER_H_

#include <string>

#include "pointer_ref.h"

namespace jcu {
namespace jvm {

class DsoHandle {
 public:
  virtual ~DsoHandle() = default;
  virtual void addDependency(PointerRef<DsoHandle>&& handle) = 0;
  virtual int open(const char* path) = 0;
  virtual bool isLoaded() const = 0;
  virtual int getErrno() const = 0;
  virtual const char* getError() const = 0;
  virtual void *getProc(const char* name) const = 0;
  virtual void close() = 0;
  virtual const char* getPath() const = 0;
};

struct JvmLibraryPathInfo {
  std::string java_home;
  std::string jvm_path;
  std::string jsig_path;
};

class OsHandler {
 public:
  virtual ~OsHandler() = default;

  virtual JvmLibraryPathInfo findJvmLibrary(const char* jvm_dll_path = nullptr, const char* java_home_path = nullptr) const = 0;

  virtual DsoHandle* createDsoHandle() const = 0;
  virtual DsoHandle* loadLibrary(DsoHandle* handle, const char* path) const = 0;

  virtual int getCurrentPid() const = 0;
  virtual int getParentPid() const = 0;

  static OsHandler* create();
};

} // namespace jvm
} // namespace jcu

#endif //JCU_JVM_OS_HANDLER_H_
