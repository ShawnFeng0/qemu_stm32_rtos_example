//
// Created by shawnfeng on 2021/11/23.
//

#pragma once

#include <stdint.h>

namespace utos {
namespace time {

uint64_t get_time_ms();
void increase(uint32_t diff_ms);

}  // namespace time
}  // namespace utos
