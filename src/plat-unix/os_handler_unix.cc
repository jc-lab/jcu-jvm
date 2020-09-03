/**
 * @file	os_library_unix.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/02
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include <jcu-jvm/os_handler.h>

#include <intl_utils.h>

#include "dso.h"
#include "location.h"

namespace jcu {
namespace jvm {

class DsoHandleUnix : public DsoHandle {
 private:
  dso_handle handle_;
  int errno_;
  std::string error_;
  std::string path_;

 public:
  DsoHandleUnix()
  : handle_(nullptr), errno_(0)
  {
  }

  ~DsoHandleUnix() override {
    close();
  }

  int open(const char *path) override {
    close();
    errno_ = 0;
    error_.clear();
    handle_ = ::INTL_CFUNC(dso_link)(path);
    if (!handle_) {
      errno_ = errno;
      const char *msg = ::INTL_CFUNC(dso_error)();
      error_.clear();
      if (msg) error_ = msg;
      return errno_;
    }
    path_ = path;
    return 0;
  }

  bool isLoaded() const override {
    return handle_ != nullptr;
  }

  int getErrno() const override {
    return errno_;
  }

  const char *getError() const override {
    return error_.c_str();
  }

  void *getProc(const char *name) const override {
    if (handle_ == nullptr)
      return nullptr;
    return ::INTL_CFUNC(dso_symbol)(handle_, name);
  }

  void close() override {
    if (handle_) {
      ::INTL_CFUNC(dso_unlink)(handle_);
      handle_ = nullptr;
    }
  }

  const char* getPath() const override {
    return path_.c_str();
  }
};

class OsHandleUnix : public OsHandler {
 public:
  OsHandleUnix() {
    INTL_CFUNC(dso_init)();
  }

  DsoHandle *createDsoHandle() const override {
    return new DsoHandleUnix();
  }

  static int loadJvmFromJavaHome(DsoHandle* dso_handle, const char* java_home_path) {
    for(char* const * it = INTL_CFUNC(location_jvm_default); *it != nullptr; it++) {
      struct stat st = { 0 };
      std::string item(*it);
      item = intl::stringReplace<char>(item, "JAVA_HOME", java_home_path);
      if (stat(item.c_str(), &st) == 0) {
        if (dso_handle->open(item.c_str()) == 0) {
          return 0;
        }
      }
    }
    return dso_handle->getErrno();
  }

  static int loadJvm(DsoHandle* dso_handle, const char* jvm_dll_path, const char* java_home_path) {
    auto jvm_dll_path_string = intl::utf8ToSystem(jvm_dll_path);
    auto java_home_path_string = intl::utf8ToSystem(java_home_path);

    if (!jvm_dll_path_string.empty()) {
      return dso_handle->open(jvm_dll_path_string.c_str());
    } else if (!java_home_path_string.empty()) {
      return loadJvmFromJavaHome(dso_handle, java_home_path_string.c_str());
    } else {
      const char* env_java_home = getenv("JAVA_HOME");
      if (env_java_home) {
        if (loadJvmFromJavaHome(dso_handle, env_java_home) == 0) {
          return 0;
        }
      }
    }

    return dso_handle->open("libjvm.so");
  }

  DsoHandle *loadJvmLibrary(DsoHandle *handle, const char *jvm_dll_path, const char *java_home_path) const override {
    if (!handle)
      handle = new DsoHandleUnix();
    loadJvm(handle, jvm_dll_path, java_home_path);
    return handle;
  }

  int getCurrentPid() const override {
    return ::getpid();
  }

  int getParentPid() const override {
    return ::getppid();
  }
};

OsHandler* OsHandler::create() {
  return new OsHandleUnix();
}

} // namespace jvm
} // namespace jcu
