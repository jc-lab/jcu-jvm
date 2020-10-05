/**
 * @file	os_library_unix.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/02
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <list>
#include <cstring>

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

  std::list<PointerRef<DsoHandle>> dependencies_;

 public:
  DsoHandleUnix()
  : handle_(nullptr), errno_(0)
  {
  }

  ~DsoHandleUnix() override {
    close();
  }

  void addDependency(PointerRef<DsoHandle>&& handle) override {
    dependencies_.emplace_back(std::move(handle));
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

  static std::string findJvmFromJavaHome(const char* java_home_path) {
    for(char* const * it = INTL_CFUNC(location_jvm_default); *it != nullptr; it++) {
      struct stat st = { 0 };
      std::string item(*it);
      item = intl::stringReplace<char>(item, "JAVA_HOME", java_home_path);
      if (stat(item.c_str(), &st) == 0) {
        return item;
      }
    }
    return "";
  }

  JvmLibraryPathInfo findJvmLibrary(const char* jvm_dll_path, const char* java_home_path) const {
    JvmLibraryPathInfo info;

    if (jvm_dll_path) {
      info.jvm_path = jvm_dll_path;
    }
    if (java_home_path) {
      info.java_home = java_home_path;
    }

    if (info.jvm_path.empty()) {
      if (!info.java_home.empty()) {
        info.jvm_path = findJvmFromJavaHome(info.java_home.c_str());
      } else {
        const char* env_java_home = getenv("JAVA_HOME");
        if (env_java_home) {
          info.java_home = env_java_home;
          info.jvm_path = findJvmFromJavaHome(env_java_home);
        }
      }
    }

    if (info.jvm_path.empty()) {
      info.jvm_path = "libjvm.so";
    }

    std::string expect_dir(info.jvm_path);
    for (int trycount = 0; trycount < 2; trycount++) {
      struct stat st = { 0 };
      const char* last_slash = std::strrchr(expect_dir.c_str(), '/');
      if (last_slash) {
        expect_dir = std::string(expect_dir.c_str(), last_slash);
        std::string temp(expect_dir);
        temp.append("/libjsig.so");
        if (::stat(temp.c_str(), &st) == 0) {
          info.jsig_path = temp;
          break;
        }
      } else {
        break;
      }
    }
    if (info.jsig_path.empty()) {
      info.jsig_path = "libjsig.so";
    }

    return std::move(info);
  }

  DsoHandle* loadLibrary(DsoHandle* handle, const char* path) const override {
    if (!handle)
      handle = new DsoHandleUnix();
    handle->open(path);
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
