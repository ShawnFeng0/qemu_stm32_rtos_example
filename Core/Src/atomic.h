/**
 * @file orb_atomic.h
 *
 * Provides atomic integers and counters. Each method is executed atomically and
 * thus can be used to prevent data races and add memory synchronization between
 * threads.
 *
 * In addition to the atomicity, each method serves as a memory barrier
 * (sequential consistent ordering). This means all operations that happen
 * before and could potentially have visible side-effects in other threads will
 * happen before the method is executed.
 *
 * The implementation uses the built-in methods from GCC (supported by Clang as
 * well).
 * @see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html.
 *
 * @note: on ARM, the instructions LDREX and STREX might be emitted. To ensure
 * correct behavior, the exclusive monitor needs to be cleared on a task switch
 * (via CLREX). This happens automatically e.g. on ARMv7-M as part of an
 * exception entry or exit sequence.
 */

#pragma once

#ifdef __cplusplus

namespace utos {

template <typename T> class atomic {
public:
  // Ensure that all operations are lock-free, so that 'atomic' can be used from
  // IRQ handlers. This might not be required everywhere though.
  static_assert(__atomic_always_lock_free(sizeof(T), nullptr),
                "atomic is not lock-free for the given type T");

  atomic() = default;
  explicit atomic(T value) : value_(value) {}

  /**
   * Atomically read the current value
   */
  inline T load() const { return __atomic_load_n(&value_, __ATOMIC_SEQ_CST); }

  /**
   * Atomically store a value
   */
  inline void store(T value) {
    __atomic_store(&value_, &value, __ATOMIC_SEQ_CST);
  }

  /**
   * Atomically add a number and return the previous value.
   * @return value prior to the addition
   */
  inline T fetch_add(T num) {
    return __atomic_fetch_add(&value_, num, __ATOMIC_SEQ_CST);
  }

  /**
   * Atomically substract a number and return the previous value.
   * @return value prior to the substraction
   */
  inline T fetch_sub(T num) {
    return __atomic_fetch_sub(&value_, num, __ATOMIC_SEQ_CST);
  }

  /**
   * Atomic AND with a number
   * @return value prior to the operation
   */
  inline T fetch_and(T num) {
    return __atomic_fetch_and(&value_, num, __ATOMIC_SEQ_CST);
  }

  /**
   * Atomic XOR with a number
   * @return value prior to the operation
   */
  inline T fetch_xor(T num) {
    return __atomic_fetch_xor(&value_, num, __ATOMIC_SEQ_CST);
  }

  /**
   * Atomic OR with a number
   * @return value prior to the operation
   */
  inline T fetch_or(T num) {
    return __atomic_fetch_or(&value_, num, __ATOMIC_SEQ_CST);
  }

  /**
   * Atomic NAND (~(_value & num)) with a number
   * @return value prior to the operation
   */
  inline T fetch_nand(T num) {
    return __atomic_fetch_nand(&value_, num, __ATOMIC_SEQ_CST);
  }

  /**
   * Atomic compare and exchange operation.
   * This compares the contents of _value with the contents of *expected. If
   * equal, the operation is a read-modify-write operation that writes desired
   * into _value. If they are not equal, the operation is a read and the current
   * contents of _value are written into *expected.
   * @return If desired is written into _value then true is returned
   */
  inline bool compare_exchange(T *expected, T num) {
    return __atomic_compare_exchange(&value_, expected, num, false,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  }

private:
  T value_{};
};

using atomic_int = atomic<int>;
using atomic_bool = atomic<bool>;

} // namespace utos

#endif /* __cplusplus */
