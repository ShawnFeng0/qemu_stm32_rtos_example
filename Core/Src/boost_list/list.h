//
// Created by shawnfeng on 2021/10/18.
//

#pragma once

#include "circular_list_algorithms.h"

namespace utos {
namespace list {
namespace internal {

template <typename T>
struct node_traits {
  using node = T;
  using node_ptr = T *;
  using const_node_ptr = const T *;

  inline static node_ptr get_next(const_node_ptr n) { return n->next_; }
  inline static void set_next(node_ptr n, node_ptr next) { n->next_ = next; }

  inline static node_ptr get_previous(const_node_ptr n) { return n->prev_; }
  inline static void set_previous(node_ptr n, node_ptr prev) {
    n->prev_ = prev;
  }
};

template <typename T>
using circular_list_algo = boost::intrusive::circular_list_algorithms<
    utos::list::internal::node_traits<T>>;

}  // namespace internal

struct node {
  node *next_;
  node *prev_;
};

using algo = utos::list::internal::circular_list_algo<node>;

}  // namespace list
}  // namespace utos
