/**
 * @file	simple_memory_pool.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/08/31
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef COMMONS_DAEMON_NATIVE_SRC_SIMPLE_MEMORY_POOL_H_
#define COMMONS_DAEMON_NATIVE_SRC_SIMPLE_MEMORY_POOL_H_

#include <set>

#include <jcu-jvm/memory_pool.h>

namespace jcu {
namespace jvm {
namespace intl {

class SimpleMemoryPool : public MemoryPool {
 private:
  std::set<void*> allocated_ptrs_;

 public:
  SimpleMemoryPool();
  ~SimpleMemoryPool();

  void* allocate(size_t size) override;
  bool release(void *ptr) override;
  void releaseAll() override;
};

} // namespace intl
} // namespace jvm
} // namespace jcu

#endif // COMMONS_DAEMON_NATIVE_SRC_SIMPLE_MEMORY_POOL_H_

