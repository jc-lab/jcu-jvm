/**
 * @file	os_library_win.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/02
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <list>
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

  std::list<PointerRef<DsoHandle>> dependencies_;

 public:
  DsoHandleWin()
      : handle_(nullptr), errno_(0) {
  }

  ~DsoHandleWin() override {
    close();
  }

  void addDependency(PointerRef<DsoHandle>&& handle) override {
    dependencies_.emplace_back(std::move(handle));
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
      std::vector<TCHAR> errmsgbuf(1024);
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

  static std::basic_string<TCHAR> findJvmFromJavaHome(const TCHAR *java_home_path) {
    TCHAR temp_path[MAX_PATH];
    size_t java_home_len = _tcslen(java_home_path);
    int rc;
    _tcscpy_s(temp_path, java_home_path);
    if (temp_path[java_home_len - 1] == '\\') {
      temp_path[--java_home_len] = 0;
    }
    _tcscat_s(temp_path, _T("\\bin\\server\\jvm.dll"));
    if (::GetFileAttributes(temp_path) != INVALID_FILE_ATTRIBUTES) {
      return temp_path;
    }

    temp_path[java_home_len] = 0;
    _tcscat_s(temp_path, _T("\\jre\\bin\\server\\jvm.dll"));
    if (::GetFileAttributes(temp_path) != INVALID_FILE_ATTRIBUTES) {
      return temp_path;
    }

    return "";
  }

  JvmLibraryPathInfo findJvmLibrary(const char* jvm_dll_path, const char* java_home_path) const {
    JvmLibraryPathInfo info;

    auto jvm_dll_path_string = intl::utf8ToSystem(jvm_dll_path);
    std::basic_string<TCHAR> jsig_dll_path_string;
    auto java_home_path_string = intl::utf8ToSystem(java_home_path);

    if (jvm_dll_path_string.empty()) {
      if (!java_home_path_string.empty()) {
        jvm_dll_path_string = findJvmFromJavaHome(info.java_home.c_str());
      } else {
        TCHAR env_java_home[MAX_PATH];
        if (::GetEnvironmentVariable(_T("JAVA_HOME"), env_java_home, MAX_PATH)) {
          java_home_path_string = env_java_home;
          jvm_dll_path_string = findJvmFromJavaHome(env_java_home);
        }
      }
    }

    if (jvm_dll_path_string.empty()) {
      jvm_dll_path_string = _T("jvm.dll");
    }

    std::string expect_dir(info.jvm_path);
    for (int trycount = 0; trycount < 2; trycount++) {
      const char* last_slash = _tcsrchr(expect_dir.c_str(), _T('\\'));
      if (!last_slash) {
        last_slash = _tcsrchr(expect_dir.c_str(), _T('/'));
      }
      if (last_slash) {
        expect_dir = std::basic_string<TCHAR>(expect_dir.c_str(), last_slash);
        std::string temp(expect_dir);
        temp.append(_T("\\jsig.dll"));
        if (::GetFileAttributes(temp.c_str()) != INVALID_FILE_ATTRIBUTES) {
          jsig_dll_path_string = temp;
          break;
        }
      } else {
        break;
      }
    }
    if (jsig_dll_path_string.empty()) {
      jsig_dll_path_string = _T("libjsig.so");
    }

    info.jvm_path = intl::systemToUtf8(jvm_dll_path_string.c_str(), jvm_dll_path_string.length());
    info.jsig_path = intl::systemToUtf8(jsig_dll_path_string.c_str(), jsig_dll_path_string.length());
    info.java_home = intl::systemToUtf8(java_home_path_string.c_str(), java_home_path_string.length());

    return std::move(info);
  }

  DsoHandle* loadLibrary(DsoHandle* handle, const char* path) const override {
    if (!handle)
      handle = new DsoHandleWin();
    handle->open(path);
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
