//
// Created by shawnfeng on 2021/12/1.
//

#pragma once

#include "event.h"

namespace utos {

class ConditionVariable {
  void notify_one() noexcept { event.notify_one(); }
  void notify_all() noexcept { event.notify_all(); }

  void wait(Mutex &lock, int32_t timeout_ms = -1) noexcept {
    {
      // Unlocking and waiting is an atomic operation
      internal::IrqLockGuard irq_lock_guard;
      lock.unlock();
      event.wait(timeout_ms);
    }
    lock.lock();
  }

 private:
  Event event;
};

}  // namespace utos
