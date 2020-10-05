/**
 * @file	jvm_library_base.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/08/31
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <jcu-jvm/jvm_library.h>
#include <intl_utils.h>

namespace jcu {
namespace jvm {

class JvmLibraryBase : public JvmLibrary {
 private:
  PointerRef<OsHandler> os_handler_;
  PointerRef<DsoHandle> dso_handle_;

  int load_errno_;
  std::string load_error_;

  fnJNI_GetDefaultJavaVMInitArgs_t fnJNI_GetDefaultJavaVMInitArgs_;
  fnJNI_CreateJavaVM_t fnJNI_CreateJavaVM_;
  fnJNI_GetCreatedJavaVMs_t fnJNI_GetCreatedJavaVMs_;
  fnJVM_DumpAllStacks_t fnJVM_DumpAllStacks_;

 public:
  jint JNI_GetDefaultJavaVMInitArgs(void *vm_args) const override {
    return fnJNI_GetDefaultJavaVMInitArgs_(vm_args);
  }

  jint JNI_CreateJavaVM(JavaVM **p_vm, JNIEnv **p_env, void *vm_args) const override {
    return fnJNI_CreateJavaVM_(p_vm, p_env, vm_args);
  }

  jint JNI_GetCreatedJavaVMs(JavaVM **vmBuf, jsize bufLen, jsize *nVMs) const override {
    return fnJNI_GetCreatedJavaVMs_(vmBuf, bufLen, nVMs);
  }

  void JVM_DumpAllStacks(JNIEnv *env, jclass cls) const override {
    fnJVM_DumpAllStacks_(env, cls);
  }

  JvmLibraryBase(PointerRef<OsHandler> &&os_handler)
      : load_errno_(0), os_handler_(std::move(os_handler))
  {
    fnJNI_GetDefaultJavaVMInitArgs_ = nullptr;
    fnJNI_CreateJavaVM_ = nullptr;
    fnJNI_GetCreatedJavaVMs_ = nullptr;
    fnJVM_DumpAllStacks_ = nullptr;
  }

  ~JvmLibraryBase() {
    close();
  }

  OsHandler* getOsHandle() const override {
    return os_handler_.get();
  }

  void close() {
    if (dso_handle_) {
      dso_handle_->close();
    }
    fnJNI_GetDefaultJavaVMInitArgs_ = nullptr;
    fnJNI_CreateJavaVM_ = nullptr;
    fnJNI_GetCreatedJavaVMs_ = nullptr;
    fnJVM_DumpAllStacks_ = nullptr;
  }

  bool isLoaded() const override {
    return dso_handle_ && dso_handle_->isLoaded();
  }

  int getLoadErrno() const override {
    return load_errno_;
  }

  const char* getLoadError() const override {
    return load_error_.c_str();
  }

  const char* getJvmPath() const override {
    return dso_handle_ ? dso_handle_->getPath() : nullptr;
  }

  int load(const JvmLibraryPathInfo& path_info, bool jsig_load = false) {
    int rc;
    PointerRef<DsoHandle> dso_handle;
    dso_handle = std::unique_ptr<DsoHandle>(os_handler_->createDsoHandle());

    if (jsig_load) {
      PointerRef<DsoHandle> jsig_handle;
      jsig_handle = std::unique_ptr<DsoHandle>(os_handler_->createDsoHandle());
      rc = jsig_handle->open(path_info.jsig_path.c_str());
      if (rc) {
        return rc;
      }
      dso_handle->addDependency(std::move(jsig_handle));
    }

    rc = dso_handle->open(path_info.jvm_path.c_str());
    if (rc) {
      return rc;
    }

    dso_handle_ = std::move(dso_handle);

    fnJNI_GetDefaultJavaVMInitArgs_ = (fnJNI_GetDefaultJavaVMInitArgs_t)dso_handle_->getProc("JNI_GetDefaultJavaVMInitArgs");
    fnJNI_CreateJavaVM_ = (fnJNI_CreateJavaVM_t)dso_handle_->getProc("JNI_CreateJavaVM");
    fnJNI_GetCreatedJavaVMs_ = (fnJNI_GetCreatedJavaVMs_t)dso_handle_->getProc("JNI_GetCreatedJavaVMs");
    fnJVM_DumpAllStacks_ = (fnJVM_DumpAllStacks_t)dso_handle_->getProc("JVM_DumpAllStacks");

    return 0;
  }
};

JvmLibrary* JvmLibrary::create(PointerRef<OsHandler> os_handler) {
  return new JvmLibraryBase(std::move(os_handler));
}

} // namespace jvm
} // namespace jcu
