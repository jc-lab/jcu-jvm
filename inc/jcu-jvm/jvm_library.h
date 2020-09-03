/**
 * @file	jvm_library.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/08/31
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef JCU_JVM_JVM_LIBRARY_H_
#define JCU_JVM_JVM_LIBRARY_H_

#include <jni.h>

#include "pointer_ref.h"
#include "os_handler.h"

namespace jcu {
namespace jvm {

typedef jint (JNICALL *fnJNI_GetDefaultJavaVMInitArgs_t)(void *vm_args);
typedef jint (JNICALL *fnJNI_CreateJavaVM_t)(JavaVM **p_vm, JNIEnv **p_env, void *vm_args);
typedef jint (JNICALL *fnJNI_GetCreatedJavaVMs_t)(JavaVM **vmBuf, jsize bufLen, jsize *nVMs);
typedef jint (JNICALL *fnJVM_DumpAllStacks_t)(JNIEnv *env, jclass cls);
typedef jint (JNICALL* fnDestroyJavaVM_t)(JavaVM *vmBuf);

class JvmLibrary {
 public:
  virtual ~JvmLibrary() {}

  virtual OsHandler* getOsHandle() const = 0;

  /**
   * Load jvm.dll or jvm.so
   * @param path null or utf8 string
   * @return system error code
   */
  virtual bool isLoaded() const = 0;
  virtual int getLoadErrno() const = 0;
  virtual const char* getLoadError() const = 0;
  virtual const char* getJvmPath() const = 0;

  virtual jint JNI_GetDefaultJavaVMInitArgs(void *vm_args) const = 0;
  virtual jint JNI_CreateJavaVM(JavaVM **p_vm, JNIEnv **p_env, void *vm_args) const = 0;
  virtual jint JNI_GetCreatedJavaVMs(JavaVM **p_vm, jsize bufLen, jsize *nVMs) const = 0;
  virtual void JVM_DumpAllStacks(JNIEnv *env, jclass cls) const = 0;

  virtual int load(const char* jvm_dll_path = nullptr, const char* java_home_path = nullptr) = 0;

  static JvmLibrary* create(PointerRef<OsHandler> os_handler);
};

} // namespace jvm
} // namespace jcu

#endif //JCU_JVM_JVM_LIBRARY_H_
