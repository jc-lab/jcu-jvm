/**
 * @file	memory_pool.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/07
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef JCU_JVM_MEMORY_POOL_H_
#define JCU_JVM_MEMORY_POOL_H_

#include <jni.h>

#include "pointer_ref.h"
#include "os_handler.h"

namespace jcu {
namespace jvm {

class MemoryPool {
 public:
  virtual void* allocate(size_t size) = 0;
  virtual bool release(void *ptr) = 0;
  virtual void releaseAll() = 0;
};

extern MemoryPool* createSimpleMemoryPool();

} // namespace jvm
} // namespace jcu

#endif //JCU_JVM_MEMORY_POOL_H_
