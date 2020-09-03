/**
 * @file	os_library_win.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/02
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <vector>
#include <windows.h>
#include <tlhelp32.h>

#include <jcu-jvm/os_handler.h>

#include <intl_utils.h>

namespace jcu {
namespace jvm {

class DsoHandleWin : public DsoHandle {
 private:
  HMODULE handle_;
  int errno_;
  std::string error_;
  std::string path_;

 public:
  DsoHandleWin()
      : handle_(nullptr), errno_(0) {
  }

  ~DsoHandleWin() override {
    close();
  }

  bool isLoaded() const override {
    return handle_ != nullptr;
  }

  int open(const char *path) override {
    auto spath = intl::utf8ToSystem(path);
    return opent(spath.c_str());
  }

  int opent(const TCHAR *path) {
    close();
    errno_ = 0;
    error_.clear();
    handle_ = ::LoadLibrary(path);
    if (!handle_) {
      DWORD eno = ::GetLastError();
      std::vector<char> errmsgbuf(1024);
      DWORD errchars = FormatMessage(
          FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
          nullptr,
          eno,
          0,
          errmsgbuf.data(),
          (DWORD) errmsgbuf.size(),
          NULL);

      errno_ = eno;
      if (errchars > 0) {
        error_ = intl::systemToUtf8(errmsgbuf.data(), errchars);
      }
      return errno_;
    }
    path_ = intl::systemToUtf8(path);
    return 0;
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
    return (void *) ::GetProcAddress(handle_, name);
  }

  void close() override {
    if (handle_) {
      ::FreeLibrary(handle_);
      handle_ = nullptr;
    }
  }

  const char *getPath() const override {
    return path_.c_str();
  }
};

class OsHandleWin : public OsHandler {
 public:
  OsHandleWin() {
  }

  DsoHandle *createDsoHandle() const override {
    return new DsoHandleWin();
  }

  static int loadJvmFromJavaHome(DsoHandleWin *dso_handle, const TCHAR *java_home_path) {
    TCHAR temp_path[MAX_PATH];
    size_t java_home_len = _tcslen(java_home_path);
    int rc;
    _tcscpy_s(temp_path, java_home_path);
    if (temp_path[java_home_len - 1] == '\\') {
      temp_path[--java_home_len] = 0;
    }
    _tcscat_s(temp_path, _T("\\bin\\server\\jvm.dll"));
    if ((rc = dso_handle->opent(temp_path)) == 0) {
      return 0;
    }

    temp_path[java_home_len] = 0;
    _tcscat_s(temp_path, _T("\\jre\\bin\\server\\jvm.dll"));
    if ((rc = dso_handle->opent(temp_path)) == 0) {
      return 0;
    }

    return rc;
  }

  static int loadJvmLibraryImpl(DsoHandleWin *dso_handle, const char *jvm_dll_path, const char *java_home_path) {
    auto jvm_dll_path_string = intl::utf8ToSystem(jvm_dll_path);
    auto java_home_path_string = intl::utf8ToSystem(java_home_path);

    if (!jvm_dll_path_string.empty()) {
      return dso_handle->open(jvm_dll_path_string.c_str());
    } else if (!java_home_path_string.empty()) {
      return loadJvmFromJavaHome(dso_handle, java_home_path_string.c_str());
    } else {
      TCHAR env_java_home[MAX_PATH];
      if (::GetEnvironmentVariable("JAVA_HOME", env_java_home, MAX_PATH) > 0) {
        if (loadJvmFromJavaHome(dso_handle, env_java_home) == 0) {
          return 0;
        }
      }
    }

    return dso_handle->opent(_T("jvm.dll"));
  }

  DsoHandle *loadJvmLibrary(DsoHandle *handle, const char *jvm_dll_path, const char *java_home_path) const override {
    DsoHandleWin *handle_impl;
    if (!handle) {
      handle_impl = new DsoHandleWin();
      handle = handle_impl;
    } else {
      handle_impl = dynamic_cast<DsoHandleWin *>(handle);
      if (handle_impl == nullptr)
        return nullptr;
    }
    loadJvmLibraryImpl(handle_impl, jvm_dll_path, java_home_path);
    return handle;
  }

  int getCurrentPid() const override {
    return ::GetCurrentProcessId();
  }

  int getParentPid() const override {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD ppid = 0, pid = GetCurrentProcessId();

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    __try {
        if ( hSnapshot == INVALID_HANDLE_VALUE) __leave;

        ZeroMemory(&pe32, sizeof(pe32));
        pe32.dwSize = sizeof( pe32 );
        if ( !Process32First( hSnapshot, &pe32 )) __leave;

        do {
          if (pe32.th32ProcessID == pid) {
            ppid = pe32.th32ParentProcessID;
            break;
          }
        } while ( Process32Next( hSnapshot, &pe32 ));
    }
    __finally {
        if ( hSnapshot != INVALID_HANDLE_VALUE) CloseHandle( hSnapshot );
    }
    return -1;
  }
};

OsHandler *OsHandler::create() {
  return new OsHandleWin();
}

} // namespace jvm
} // namespace jcu
