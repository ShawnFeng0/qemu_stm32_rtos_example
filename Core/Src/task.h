#pragma once

#include <cstdint>

#include "common.h"
#include "intrusive_list/list.h"

namespace utos {

struct Task {
  uint32_t *stack;
  uint32_t *stack_raw;
  uint32_t priority;
  intrusive_list::list_node task_node;
};

using TaskList = intrusive_list::list<utos::Task, &utos::Task::task_node>;

typedef void *(*task_entry_t)(void *);

}  // namespace utos

extern utos::Task *utos_current_task;

utos::Task *task_init(utos::task_entry_t task_entry, void *parameters,
                      uint32_t priority, uint32_t stack_size);

UTOS_EXPORT void start_scheduler();
UTOS_EXPORT void task_scheduling();
