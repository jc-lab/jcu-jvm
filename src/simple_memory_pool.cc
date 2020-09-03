/**
 * @file	simple_memory_pool.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/08/31
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <stdlib.h>

#include "simple_memory_pool.h"

namespace jcu {
namespace jvm {
namespace intl {

SimpleMemoryPool::SimpleMemoryPool() {
}

SimpleMemoryPool::~SimpleMemoryPool() {
  releaseAll();
}

void* SimpleMemoryPool::allocate(size_t size) {
  void *ptr = ::malloc(size);
  allocated_ptrs_.insert(ptr);
  return ptr;
}

bool SimpleMemoryPool::release(void *ptr) {
  auto it = allocated_ptrs_.find(ptr);
  if (it != allocated_ptrs_.cend()) {
    allocated_ptrs_.erase(ptr);
    ::free(ptr);
    return true;
  }
  return false;
}

void SimpleMemoryPool::releaseAll() {
  for (auto it = allocated_ptrs_.begin(); it != allocated_ptrs_.end(); ) {
    ::free(*it);
    it = allocated_ptrs_.erase(it);
  }
}

} // namespace intl
} // namespace jvm
} // namespace jcu
