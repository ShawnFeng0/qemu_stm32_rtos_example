//
// Created by shawnfeng on 2021/11/23.
//

#include "tick.h"

#include "common.h"

static uint64_t timestamp_ms_;

uint64_t utos::time::get_time_ms() {
  internal::IrqLockGuard irq_lock_guard;
  return timestamp_ms_;
}

void utos::time::increase(uint32_t diff_ms) {
  internal::IrqLockGuard irq_lock_guard;
  timestamp_ms_ += diff_ms;
}
