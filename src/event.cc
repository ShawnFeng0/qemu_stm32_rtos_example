//
// Created by shawnfeng on 2021/11/23.
//

#include "utos/event.h"

#include "utos/tick.h"

bool utos::Event::wait(int32_t timeout_ms) {
  Task *current_task = utos_task_current();

  {
    internal::IrqLockGuard irq_lock_guard;

    utos_task_mark_unready(current_task);

    if (timeout_ms >= 0) {
      utos_task_register_timeout(
          current_task,
          [](Task *t, void *cb_data) {
            auto *event = static_cast<Event *>(cb_data);
            event->remove_from_wait_list(t);
            utos_task_mark_ready(t);
            t->event_result = Task::EventResult::TIMEOUT;
          },
          this, time::get_time_ms() + timeout_ms);
    }

    this->add_to_wait_list(current_task);
  }

  utos_task_yield();

  return current_task->event_result == Task::EventResult::RECEIVED;
}

void utos::Event::notify_one() {
  internal::IrqLockGuard irq_lock_guard;

  if (!event_list.empty()) {
    decltype(auto) task = event_list.front();
    event_list.pop_front();

    utos_task_unregister_timeout(&task);
    utos_task_mark_ready(&task);

    task.event_result = Task::EventResult::RECEIVED;
  }
}

void utos::Event::notify_all() {
  internal::IrqLockGuard irq_lock_guard;

  while (!event_list.empty()) {
    decltype(auto) task = event_list.front();
    event_list.pop_front();

    utos_task_unregister_timeout(&task);
    utos_task_mark_ready(&task);

    task.event_result = Task::EventResult::RECEIVED;
  }
}
