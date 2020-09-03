/**
 * @file	vm.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/01
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <stdarg.h>

#include <jcu-jvm/pointer_ref.h>
#include <jcu-jvm/vm.h>

#include <intl_utils.h>

#include "simple_memory_pool.h"

namespace jcu {
namespace jvm {

extern "C" {

static void _java_exit(int x) {
  exit(x);
}

static void _java_abort123() {
  exit(123);
}

static void _java_vfprintf(FILE* s, const char* format, va_list args) {
  vfprintf(s, format, args);
}

}

class VMImpl : public VM {
 public:
  PointerRef<JvmLibrary> jvm_library_;
  OsHandler* os_handler_;

  JavaVM* jvm_;
  JNIEnv* env_;

  jclass cls_system_;

  VMImpl(PointerRef<JvmLibrary>&& jvm_library) {
    jvm_library_ = std::move(jvm_library);
    os_handler_ = jvm_library_->getOsHandle();
    clear();
  }

  ~VMImpl() {
    destroy();
  }

  void clear() {
    jvm_ = nullptr;
    env_ = nullptr;
    cls_system_ = nullptr;
  }

  jint init() override {
    intl::SimpleMemoryPool regional_pool;
    JavaVMInitArgs init_args = { 0 };
    int opt;
    jint rc;

    destroy();

    init_args.version = JNI_VERSION_1_8;
    init_args.ignoreUnrecognized = JNI_TRUE;

    init_args.nOptions = 5;
    init_args.options = (JavaVMOption*)regional_pool.allocate(sizeof(JavaVMOption) * init_args.nOptions);

    opt = 0;
    {
      JavaVMOption* item = &init_args.options[opt++];
      item->optionString = intl::mpollStrdup(&regional_pool, "exit");
      item->extraInfo = (void*)_java_exit;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      item->optionString = intl::mpollStrdup(&regional_pool, "abort");
      item->extraInfo = (void*)_java_abort123;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      item->optionString = intl::mpollStrdup(&regional_pool, "vfprintf");
      item->extraInfo = (void*)_java_vfprintf;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      char* buffer = (char*)regional_pool.allocate(128);
      snprintf(buffer, 128, "-Djcu.jvm.process.ppid=%d", os_handler_->getParentPid());
      item->optionString = buffer;
      item->extraInfo = (void*) nullptr;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      char* buffer = (char*)regional_pool.allocate(128);
      snprintf(buffer, 128, "-Djcu.jvm.process.pid=%d", os_handler_->getCurrentPid());
      item->optionString = buffer;
      item->extraInfo = (void*) nullptr;
    }

    rc = jvm_library_->JNI_CreateJavaVM(&jvm_, &env_, &init_args);

    cls_system_ = env_->FindClass("java/lang/System");

    return rc;
  }

  void callExit(jint code) {
    jmethodID method = env_->GetStaticMethodID(cls_system_, "exit", "(I)V");
    env_->CallStaticVoidMethod(cls_system_, method, code);
  }

  jint destroy() override {
    jint rc = -1;
    if (jvm_) {
      rc = jvm_->DestroyJavaVM();
    }
    clear();
    return rc;
  }


  virtual JavaVM* jvm() const override {
    return jvm_;
  }
  virtual JNIEnv* env() const override {
    return env_;
  }
};

VM* VM::create(PointerRef<JvmLibrary> jvm_library) {
  return new VMImpl(std::move(jvm_library));
}

} // namespace jvm
} // namespace jcu
