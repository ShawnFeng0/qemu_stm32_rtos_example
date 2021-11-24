#pragma once

#include <cstdint>

#include "common.h"
#include "intrusive_list/list.h"

namespace utos {

using WaitCallback = void(void *user_data);

struct Task : internal::Noncopyable {
  uint32_t *stack;
  uint32_t *stack_raw;
  uint32_t priority;
  intrusive_list::list_node task_node;

  enum class State {
    RUNNING,
    READY,
    BLOCK,
  } state;

  intrusive_list::list_node event_node;
  enum class EventResult {
    RECEIVED,
    TIMEOUT,
  } event_result;

  intrusive_list::list_node timeout_node;
  uint64_t timeout_deadline_ms;
  WaitCallback timeout_cb;
  void *timeout_cb_data;
};

using TaskList = intrusive_list::list<utos::Task, &utos::Task::task_node>;

typedef void *(*task_entry_t)(void *);

}  // namespace utos

extern utos::Task *utos_current_task;

utos::Task *task_init(utos::task_entry_t task_entry, void *parameters,
                      uint32_t priority, uint32_t stack_size);

void task_mark_unready(utos::Task *task);
void task_mark_ready(utos::Task *task);

UTOS_EXPORT void start_scheduler();

void task_yield();
