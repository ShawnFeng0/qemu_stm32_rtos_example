//
// Created by shawnfeng on 2021/8/20.
//

#include "utos/task.h"

#include <cstdio>
#include <cstring>

#include "int_bitmap.h"
#include "intrusive_list/list.h"
#include "utos/event.h"
#include "utos/tick.h"

// Need to be implemented externally
void utos_port_start_first_task();
uint32_t *utos_port_task_stack_init(utos::task_entry_t task_entry,
                                    void *parameters, uint32_t *top_of_stack);
void utos_port_task_yield();

static utos::Task *utos_idle_task;
[[noreturn]] static void *utos_idle_task_entry(void *) {
  for (;;) {
  }
}

utos::Task *utos_current_task;

static utos::IntBitmap utos_task_priority_bitmap;
static constexpr auto kUtosMaxPriority = utos::IntBitmap::count() - 1;
static constexpr auto kUtosMinPriority = 0;
static intrusive_list::list<utos::Task, &utos::Task::ready_list_node>
    utos_ready_task_list[kUtosMaxPriority + 1];

static intrusive_list::list<utos::Task, &utos::Task::timeout_list_node>
    utos_timeout_task_list;

static inline utos::Task *create_task(const char *name,
                                      void *(*task_entry)(void *),
                                      void *parameters, uint32_t priority,
                                      uint32_t stack_size) {
  auto *task = new utos::Task;
  task->stack_raw = new uint32_t[stack_size];

  strlcpy(task->name, name, sizeof(task->name));

  task->ready_list_node.task = task;
  task->event_list_node.task = task;
  task->timeout_list_node.task = task;

  task->stack = utos_port_task_stack_init(task_entry, parameters,
                                          task->stack_raw + stack_size);
  task->priority = priority <= kUtosMaxPriority ? priority : kUtosMaxPriority;
  return task;
}

void utos_start_scheduler() {
  utos_idle_task = create_task("idle", utos_idle_task_entry, nullptr, 0, 128);
  utos_task_scheduling();
  utos_port_start_first_task();
}

utos::Task *utos_task_create(const char *name, utos::task_entry_t task_entry,
                             void *parameters, uint32_t priority,
                             uint32_t stack_size) {
  utos::Task *task =
      create_task(name, task_entry, parameters, priority, stack_size);
  utos_task_ready(task);
  return task;
}

int utos_max_priority() { return kUtosMaxPriority; }
int utos_min_priority() { return kUtosMinPriority; }

void utos_task_scheduling() {
  utos::internal::IrqLockGuard irq_lock_guard;

  if (utos_task_priority_bitmap.valid()) {
    auto highest_ready = utos_task_priority_bitmap.find_first_set();
    utos_ready_task_list[highest_ready].rotate_left();
    utos_current_task = &utos_ready_task_list[highest_ready].front();
  } else {
    utos_current_task = utos_idle_task;
  }
}

void utos_task_ready(utos::Task *task) {
  utos::internal::IrqLockGuard irq_lock_guard;
  utos_ready_task_list[task->priority].push_back(*task);
  utos_task_priority_bitmap.set(task->priority);
  task->state = utos::Task::State::READY;
}

void utos_task_suspend(utos::Task *task) {
  utos::internal::IrqLockGuard irq_lock_guard;
  utos_ready_task_list[task->priority].remove_if_exists(*task);

  if (utos_ready_task_list[task->priority].empty())
    utos_task_priority_bitmap.clear(task->priority);
  task->state = utos::Task::State::SUSPEND;
}

void utos_task_register_timeout(utos::Task *task,
                                utos::Task::TimeoutCallback cb, void *cb_data,
                                uint64_t deadline_time_ms) {
  utos::internal::IrqLockGuard irq_lock_guard;

  task->timeout_cb = cb;
  task->timeout_cb_data = cb_data;
  task->timeout_deadline_ms = deadline_time_ms;

  utos_timeout_task_list.push_back(*task);
}

void utos_task_unregister_timeout(utos::Task *task) {
  utos::internal::IrqLockGuard irq_lock_guard;
  utos_timeout_task_list.remove_if_exists(*task);
}

void utos_task_system_tick_handler(uint32_t diff_ms) {
  {
    utos::internal::IrqLockGuard irq_lock_guard;
    utos::time::increase(diff_ms);
    for (auto i = utos_timeout_task_list.begin();
         i != utos_timeout_task_list.end();) {
      auto timeout = i->timeout_deadline_ms;
      if (utos::time::get_time_ms() >= timeout) {
        i->timeout_cb(&(*i), i->timeout_cb_data);
        i = utos_timeout_task_list.erase(i);
      } else {
        ++i;
      }
    }
  }

  if (utos_current_task) utos_task_yield();
}

void utos_task_yield() { utos_port_task_yield(); }

void utos_task_sleep(uint32_t timeout_ms) {
  {
    utos::internal::IrqLockGuard irq_lock_guard;
    auto task = utos_current_task;
    utos_task_suspend(task);
    utos_task_register_timeout(
        task, [](utos::Task *t, void *unused) { utos_task_ready(t); },
        nullptr, utos::time::get_time_ms() + timeout_ms);
  }

  utos_task_yield();
}

utos::Task *utos_task_current() { return utos_current_task; }
