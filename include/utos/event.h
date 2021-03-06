//
// Created by shawnfeng on 2021/11/12.
//

#pragma once

#include "intrusive_list/list.h"
#include "task.h"

namespace utos {

class Event : internal::Noncopyable {
 public:
  /**
   * Wait for the event to occur
   * @param task Current task
   * @param timeout_ms
   *            Timeout time, relative to the current, unit: milliseconds.
   *            If it is less than 0, wait forever
   * @return true if the event occurs
   * @return false otherwise
   */
  bool wait(int32_t timeout_ms = -1);

  void notify_one();
  void notify_all();

 private:
  void add_to_wait_list(Task *task) { event_list.push_back(*task); }
  void remove_from_wait_list(Task *task) { event_list.remove_if_exists(*task); }

  using EventList = intrusive_list::list<Task, &Task::event_list_node>;
  EventList event_list;
};

}  // namespace utos
