//
// Created by shawnfeng on 2021/11/12.
//

#include "intrusive_list/list.h"
#include "task.h"

namespace utos {

class Event : internal::Noncopyable {
 public:
  void wait(Task *task, int32_t timeout_ms);

  void notify_one();
  void notify_all();

 private:
  using EventList = intrusive_list::list<Task, &Task::event_node>;
  EventList event_list;
};

}  // namespace utos
