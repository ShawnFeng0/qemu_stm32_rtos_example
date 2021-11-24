//
// Created by shawnfeng on 2021/8/20.
//

#include "task.h"

#include "event.h"
#include "int_bitmap.h"
#include "intrusive_list/list.h"
#include "tick.h"

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

static utos::TaskList utos_ready_task_list;
static utos::TaskList utos_timeout_task_list;

static constexpr auto kUtosMaxPriority = utos::IntBitmap::count() - 1;
static utos::IntBitmap utos_task_priority_bitmap;

static inline utos::Task *create_task(void *(*task_entry)(void *),
                                      void *parameters, uint32_t priority,
                                      uint32_t stack_size) {
  auto *task = new utos::Task;
  task->stack_raw = new uint32_t[stack_size];

  task->stack = utos_port_task_stack_init(task_entry, parameters,
                                          task->stack_raw + stack_size);
  task->priority = priority <= kUtosMaxPriority ? priority : kUtosMaxPriority;
  return task;
}

static inline utos::Task *task_register(utos::Task *task) {
  utos::internal::IrqLockGuard irq_lock_guard;
  utos_ready_task_list.push_back(*task);
  utos_task_priority_bitmap.set(task->priority);
  return task;
}

void utos_start_scheduler() {
  utos_idle_task = create_task(utos_idle_task_entry, nullptr, 0, 128);

  // TODO: Need init, not idle task
  utos_current_task = utos_idle_task;

  utos_port_start_first_task();
}

utos::Task *utos_task_init(utos::task_entry_t task_entry, void *parameters,
                      uint32_t priority, uint32_t stack_size) {
  return task_register(
      create_task(task_entry, parameters, priority, stack_size));
}

UTOS_EXPORT void task_scheduling() {
  utos::internal::IrqLockGuard irq_lock_guard;

  if (utos_ready_task_list.empty()) {
    utos_current_task = utos_idle_task;
  } else {
    utos_ready_task_list.rotate_left();
    utos_current_task = &utos_ready_task_list.front();
  }
}

void utos_task_mark_unready(utos::Task *task) { utos_ready_task_list.remove(*task); }
void utos_task_mark_ready(utos::Task *task) {
  utos_ready_task_list.push_back(*task);
}

void utos_task_yield() { utos_port_task_yield(); }
