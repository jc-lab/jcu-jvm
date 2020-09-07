/**
 * @file	vm.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/01
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef JCU_JVM_VM_H_
#define JCU_JVM_VM_H_

#include "pointer_ref.h"
#include "jvm_library.h"
#include "memory_pool.h"

namespace jcu {
namespace jvm {

class VM {
 public:
  virtual jint init(const char* classpath, const JavaVMInitArgs* init_args = nullptr, MemoryPool* mpool = nullptr) = 0;
  virtual jint destroy() = 0;

  virtual JavaVM* jvm() const = 0;
  virtual JNIEnv* env() const = 0;

  virtual void callExit(jint code) = 0;

  virtual jint attachThread(bool* attached) = 0;
  virtual jint attachThreadEnv(JNIEnv** env, bool* attached) = 0;
  virtual jint detachThread() = 0;

  static VM* create(PointerRef<JvmLibrary> jvm_library);
};

} // namespace jvm
} // namespace jcu

#endif //JCU_JVM_VM_H_
