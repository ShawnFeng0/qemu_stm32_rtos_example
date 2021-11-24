//
// Created by shawnfeng on 2021/10/20.
//

#pragma once

#include <atomic>

#include "cmsis_gcc.h"

#ifdef __cplusplus
#define UTOS_EXPORT extern "C"
#else
#define UTOS_EXPORT
#endif

#define UTOS_ASM(...) __asm volatile(#__VA_ARGS__)
#define UTOS_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

namespace utos {
namespace internal {

/**
 * Automatically turn off and turn on interrupts
 *
 * @code
 * {
 *  IrqLockGuard irq_lock_guard;
 *  // some code here
 * }
 * @endcode
 */
class IrqLockGuard {
 public:
  IrqLockGuard() {
    __disable_irq();
    __DSB();
    __ISB();
    ++critical_nesting();
  }
  ~IrqLockGuard() {
    if (--critical_nesting() == 0) {
      __enable_irq();
    }
  }

 private:
  static inline std::atomic<uint32_t>& critical_nesting() {
    static std::atomic<uint32_t> critical_nesting_{0};
    return critical_nesting_;
  }
};

class Noncopyable {
 public:
  Noncopyable() = default;
  ~Noncopyable() = default;

  Noncopyable(const Noncopyable&) = delete;
  Noncopyable& operator=(const Noncopyable&) = delete;
};

}  // namespace internal
}  // namespace utos
