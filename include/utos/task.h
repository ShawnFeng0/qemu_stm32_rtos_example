#pragma once

#include <cstdint>

#include "common.h"
#include "intrusive_list/list.h"

namespace utos {

struct Task;
struct TaskNode {
  TaskNode *next;
  TaskNode *prev;
  Task *task;  // For debug
};

struct Task : internal::Noncopyable {
  using WaitCallback = void(Task *, void *user_data);

  uint32_t *stack;
  uint32_t *stack_raw;
  uint32_t priority;
  char name[16];
  TaskNode node_for_ready_list;

  enum class State {
    RUNNING,
    READY,
    BLOCK,
  } state;

  TaskNode node_for_event_list;
  enum class EventResult {
    RECEIVED,
    TIMEOUT,
  } event_result;

  TaskNode node_for_timeout_list;
  uint64_t timeout_deadline_ms;
  WaitCallback *timeout_cb;
  void *timeout_cb_data;
};

typedef void *(*task_entry_t)(void *);

}  // namespace utos

extern utos::Task *utos_current_task;

utos::Task *utos_task_create(const char *name, utos::task_entry_t task_entry,
                             void *parameters, uint32_t priority,
                             uint32_t stack_size);
int utos_max_priority();
int utos_min_priority();

utos::Task *utos_task_current();

void utos_task_mark_unready(utos::Task *task);
void utos_task_mark_ready(utos::Task *task);
void utos_start_scheduler();
void utos_task_yield();

// For sleep
void utos_task_register_timeout(utos::Task *task, utos::Task::WaitCallback cb,
                                void *cb_data, uint64_t deadline_time_ms);
void utos_task_unregister_timeout(utos::Task *task);
void utos_task_sleep(uint32_t timeout_ms);

void utos_task_system_tick_handler(uint32_t diff_ms);

// Used for assembly code call
UTOS_EXPORT void utos_task_scheduling();
