//
// Created by shawnfeng on 2021/11/23.
//

#include "event.h"

#include "tick.h"

void utos::Event::wait(Task *task, int32_t timeout_ms) {
  {
    internal::IrqLockGuard irq_lock_guard;

    utos_task_mark_unready(task);
    event_list.push_back(*task);
    task->timeout_deadline_ms = time::get_time_ms() + timeout_ms;
  }

  utos_task_yield();
}

void utos::Event::notify_one() {
  internal::IrqLockGuard irq_lock_guard;

  if (!event_list.empty()) {
    decltype(auto) task = event_list.front();
    event_list.pop_front();

    utos_task_mark_ready(&task);
  }
}

void utos::Event::notify_all() {
  internal::IrqLockGuard irq_lock_guard;

  while (!event_list.empty()) {
    decltype(auto) task = event_list.front();
    event_list.pop_front();

    utos_task_mark_ready(&task);
  }
}
