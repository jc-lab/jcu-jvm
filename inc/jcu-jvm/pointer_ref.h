/**
 * @file	pointer_ref.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2020/09/01
 * @copyright Copyright (C) 2020 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef JCU_JVM_POINTER_REF_H_
#define JCU_JVM_POINTER_REF_H_

#include <memory>

namespace jcu {
namespace jvm {

template<typename T>
class PointerRef {
 private:
  std::unique_ptr<T> unique_;
  std::shared_ptr<T> shared_;
  T* native_;

 public:
  PointerRef()
  {
    native_ = nullptr;
  }

  PointerRef(std::unique_ptr<T>&& u)
  {
    unique_ = std::move(u);
    native_ = unique_.get();
  }

  PointerRef(std::shared_ptr<T> s)
  {
    shared_ = s;
    native_ = shared_.get();
  }

  PointerRef(T* p)
  {
    native_ = p;
  }

  T& operator*() const {
    return *native_;
  }

  T* operator->() const {
    return native_;
  }

  operator bool() const {
    return native_ != nullptr;
  }

  bool operator!() const {
    return native_ == nullptr;
  }

  T* get() const {
    return native_;
  }
};

} // namespace jvm
} // namespace jcu

#endif //JCU_JVM_POINTER_REF_H_
