//
// Created by shawnfeng on 2021/10/19.
//

#pragma once

namespace utos {

class IntBitmap {
 public:
  IntBitmap() noexcept : bits_(0) {}

  static constexpr uint32_t count() noexcept {
    return sizeof(IntBitmap::bits_) * 8;
  }

  inline void set(uint32_t pos) { bits_ |= 1 << pos; }

  inline void clear(uint32_t pos) { bits_ &= ~(1 << pos); }

  [[nodiscard]] bool valid() const { return bits_; }

  /**
   * @note if bits_ must greater than 0 (must valid)
   * @return return the most significant bit position
   */
  [[nodiscard]] inline uint32_t find_first_set() const {
    return count() - 1 - clz();
  }

 private:
  [[nodiscard]] inline uint32_t clz() const {
    return bits_ ? __builtin_clz(bits_) : count();
  }
  uint32_t bits_;
};

}  // namespace utos
