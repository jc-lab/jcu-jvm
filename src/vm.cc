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
  jint jni_ver_;

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
    jni_ver_ = 0;
  }

  jint init(const char* classpath, const JavaVMInitArgs* custom_init_args, MemoryPool* mpool) override {
    std::unique_ptr<intl::SimpleMemoryPool> allocated_pool;
    JavaVMInitArgs init_args = { 0 };
    int opt;
    jint rc;

    destroy();

    if (!mpool) {
      allocated_pool.reset(new intl::SimpleMemoryPool());
      mpool = allocated_pool.get();
    }

    init_args.nOptions = 5;

    if (classpath) {
      init_args.nOptions++;
    }
    if (custom_init_args) {
      init_args.version = custom_init_args->version;
      init_args.nOptions += custom_init_args->nOptions;
    }
    if (!init_args.version) {
      init_args.version = JNI_VERSION_1_8;
    }

    jvm_library_->JNI_GetDefaultJavaVMInitArgs((void*)&init_args);
    init_args.ignoreUnrecognized = JNI_TRUE;

    jni_ver_ = init_args.version;

    init_args.options = (JavaVMOption*)mpool->allocate(sizeof(JavaVMOption) * init_args.nOptions);

    opt = 0;
    if (custom_init_args) {
      for (int i = 0; i < custom_init_args->nOptions; i++) {
        JavaVMOption* item = &init_args.options[opt++];
        const JavaVMOption* src = &custom_init_args->options[i];
        item->optionString = intl::mpollStrdup(mpool, src->optionString);
        item->extraInfo = (void*)src->extraInfo;
      }
    }
    if (classpath) {
      JavaVMOption* item = &init_args.options[opt++];
      std::string temp;
      temp = "-Djava.class.path=";
      temp.append(classpath);
      item->optionString = intl::mpollStrdup(mpool, temp.c_str());
      item->extraInfo = nullptr;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      item->optionString = intl::mpollStrdup(mpool, "exit");
      item->extraInfo = (void*)_java_exit;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      item->optionString = intl::mpollStrdup(mpool, "abort");
      item->extraInfo = (void*)_java_abort123;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      item->optionString = intl::mpollStrdup(mpool, "vfprintf");
      item->extraInfo = (void*)_java_vfprintf;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      char* buffer = (char*)mpool->allocate(128);
      snprintf(buffer, 128, "-Djcu.jvm.process.ppid=%d", os_handler_->getParentPid());
      item->optionString = buffer;
      item->extraInfo = (void*) nullptr;
    }
    {
      JavaVMOption* item = &init_args.options[opt++];
      char* buffer = (char*)mpool->allocate(128);
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

  jint attachThread(bool* attached) override {
    return attachThreadEnv(&env_, attached);
  }

  jint attachThreadEnv(JNIEnv** env, bool* attached) override {
    jint rc = jvm_->GetEnv((void**)env, jni_ver_);
    if (rc != JNI_OK) {
      if (rc == JNI_EDETACHED) {
        rc = jvm_->AttachCurrentThread((void**)env, nullptr);
        if (attached) *attached = true;
      }
    }
    if (rc != JNI_OK) {
      *env = nullptr;
      return false;
    }
    return rc;
  }

  jint detachThread() override {
    return jvm_->DetachCurrentThread();
  }
};

VM* VM::create(PointerRef<JvmLibrary> jvm_library) {
  return new VMImpl(std::move(jvm_library));
}

} // namespace jvm
} // namespace jcu
