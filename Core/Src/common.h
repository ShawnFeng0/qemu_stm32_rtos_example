//
// Created by shawnfeng on 2021/10/20.
//

#pragma once

#include "cmsis_gcc.h"

#ifdef __cplusplus
#define UTOS_EXPORT extern "C"
#else
#define UTOS_EXPORT
#endif

#define UTOS_ASM(...) __asm volatile(#__VA_ARGS__)
#define UTOS_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

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
    ++critical_nesting();
  }
  ~IrqLockGuard() {
    if (--critical_nesting() == 0) __enable_irq();
  }

 private:
  static inline uint32_t& critical_nesting() {
    static uint32_t critical_nesting_;
    return critical_nesting_;
  }
};
