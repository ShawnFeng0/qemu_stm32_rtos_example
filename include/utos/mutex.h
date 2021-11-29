//
// Created by shawnfeng on 2021/11/30.
//

#pragma once

#include "utos/common.h"
#include "utos/event.h"

namespace utos {

class Mutex {
 public:
  Mutex() {
    owner = nullptr;
    owner_original_priority = 0;
    locked = false;
  }

  int32_t lock() {
    while (true) {
      internal::IrqLockGuard irq_lock_guard;

      if (!locked) {
        owner = utos_current_task;
        owner_original_priority = utos_current_task->priority;
        locked = true;

        return 0;

      } else {
        // Single thread deadlock
        if (owner == utos_current_task) {
          return -1;

        } else {  // Wait for another thread to unlock
          // Priority inversion
          if (owner->priority < utos_current_task->priority) {
            if (owner->state == Task::State::READY) {
              utos_task_suspend(utos_current_task);
              owner->priority = utos_current_task->priority;
              utos_task_ready(utos_current_task);
            } else {
              owner->priority = utos_current_task->priority;
            }
          }

          event.wait();
        }
      }
    }
  }

  int unlock() {
    internal::IrqLockGuard irq_lock_guard;

    if (!locked) {
      return 0;
    }

    if (owner != utos_current_task) {
      return -1;
    }

    locked = false;

    if (owner_original_priority != owner->priority) {
      if (owner->state == Task::State::READY) {
        utos_task_suspend(owner);
        owner->priority = owner_original_priority;
        utos_task_ready(owner);
      } else {
        owner->priority = owner_original_priority;
      }
    }

    event.notify_one();

    return 0;
  }

 private:
  Event event;
  bool locked{};
  Task *owner;
  uint32_t owner_original_priority;
};
}  // namespace utos
