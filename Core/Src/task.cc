//
// Created by shawnfeng on 2021/8/20.
//

#include "task.h"

#include <stm32f407xx.h>
#include <stm32f4xx_hal.h>

#include "int_bitmap.h"
#include "intrusive_list/list.h"

[[noreturn]] void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (true) {
  }
  /* USER CODE END Error_Handler_Debug */
}

static utos::Task *utos_idle_task;
[[noreturn]] static void *utos_idle_task_entry(void *) {
  for (;;) {
  }
}

utos::Task *utos_current_task;

static utos::TaskList utos_tasks_list;

static constexpr auto kUtosMaxPriority = utos::IntBitmap::count() - 1;
static utos::IntBitmap utos_task_priority_bitmap;

#define UTOS_ASM(...) __asm volatile(#__VA_ARGS__)

static void StartFirstTask(void) __attribute__((naked));
static void StartFirstTask(void) {
  /* Use the NVIC offset register to locate the stack. */
  UTOS_ASM(ldr r0, = 0xE000ED08);
  UTOS_ASM(ldr r0, [r0]);
  UTOS_ASM(ldr r0, [r0]);

  /* Set the msp back to the start of the stack. */
  UTOS_ASM(msr msp, r0);

  /* Clear the bit that indicates the FPU is in use */
  UTOS_ASM(mov r0, #0);
  UTOS_ASM(msr control, r0);

  /* Globally enable interrupts. */
  UTOS_ASM(cpsie i);
  UTOS_ASM(cpsie f);
  UTOS_ASM(dsb);
  UTOS_ASM(isb);

  /* System call to start first task. */
  UTOS_ASM(svc 0);
  UTOS_ASM(nop);
}

static inline uint32_t *task_stack_init(utos::task_entry_t task_entry,
                                        void *parameters,
                                        uint32_t *top_of_stack) {
  *--top_of_stack = 0x01000000;                           // xPSR register
  *--top_of_stack = (uint32_t)task_entry & 0xfffffffeUL;  // PC register
  *--top_of_stack = (uint32_t)Error_Handler;              // LR register

  top_of_stack -= 4;  // R12 R3 R2 R1

  *--top_of_stack = (uint32_t)parameters;             // R0
  *--top_of_stack = (uint32_t)EXC_RETURN_THREAD_PSP;  // exec return value

  top_of_stack -= 8;  // R11, R10, R9, R8, R7, R6, R5, R4

  return top_of_stack;
}

static inline utos::Task *create_task(void *(*task_entry)(void *),
                                      void *parameters, uint32_t priority,
                                      uint32_t stack_size) {
  auto *task = new utos::Task;
  task->stack_raw = new uint32_t[stack_size];
  task->stack =
      task_stack_init(task_entry, parameters, task->stack_raw + stack_size);
  task->priority = priority <= kUtosMaxPriority ? priority : kUtosMaxPriority;
  return task;
}

static inline utos::Task *task_register(utos::Task *task) {
  IrqLockGuard irq_lock_guard;
  utos_tasks_list.push_back(*task);
  utos_task_priority_bitmap.set(task->priority);
  return task;
}

void start_scheduler() {
  utos_idle_task = create_task(utos_idle_task_entry, nullptr, 0, 128);

  // TODO: Need init, not idle task
  utos_current_task = utos_idle_task;

  NVIC_SetPriority(PendSV_IRQn, 0xff);
  __set_CONTROL(0x3);
  __ISB();
  StartFirstTask();
}

utos::Task *task_init(utos::task_entry_t task_entry, void *parameters,
                      uint32_t priority, uint32_t stack_size) {
  return task_register(
      create_task(task_entry, parameters, priority, stack_size));
}

void task_scheduling() {
  IrqLockGuard irq_lock_guard;

  if (utos_tasks_list.empty()) {
    utos_current_task = utos_idle_task;
  } else {
    utos_tasks_list.rotate_left();
    utos_current_task = &utos_tasks_list.front();
  }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
UTOS_EXPORT void SVC_Handler(void) __attribute__((naked));
void SVC_Handler(void) {
  // Load context
  UTOS_ASM(ldr r0, curr_task_svc);
  UTOS_ASM(ldr r0, [r0]);
  UTOS_ASM(ldr r0, [r0]);
  UTOS_ASM(ldmia r0 !, {r4 - r11, r14});
  UTOS_ASM(msr psp, r0);
  UTOS_ASM(bx r14);

  // Define C data
  UTOS_ASM(.align 4);
  UTOS_ASM(curr_task_svc :.word utos_current_task);
}

/**
 * @brief This function handles Pendable request for system service.
 */
UTOS_EXPORT void PendSV_Handler(void) __attribute__((naked));
void PendSV_Handler(void) {
  // Save current context
  UTOS_ASM(mrs r0, psp);  // get stack pointer

  UTOS_ASM(ldr r3, curr_task_pend_sv);
  UTOS_ASM(ldr r2, [r3]);

  // Save fpu registers
  //  UTOS_ASM(tst r14, #0x10);
  //  UTOS_ASM(it eq);
  //  UTOS_ASM(vstmdbeq r0 !, {s16 - s31});

  UTOS_ASM(stmdb r0 !, {r4 - r11, r14});  // push to stack
  UTOS_ASM(str r0, [r2]);                 // store new psp pointer

  UTOS_ASM(stmdb sp !, {r0, r3});
  UTOS_ASM(bl task_scheduling);
  UTOS_ASM(ldmia sp !, {r0, r3});

  // Load next context
  UTOS_ASM(ldr r1, [r3]);
  UTOS_ASM(ldr r0, [r1]);

  UTOS_ASM(ldmia r0 !, {r4 - r11, r14});

  // Load fpu registers
  //  UTOS_ASM(tst r14, #0x10);
  //  UTOS_ASM(it eq);
  //  UTOS_ASM(vldmiaeq r0 !, {s16 - s31});

  UTOS_ASM(msr psp, r0);
  UTOS_ASM(bx lr);

  // Define C data
  UTOS_ASM(.align 4);
  UTOS_ASM(curr_task_pend_sv :.word utos_current_task);
}

/**
 * @brief This function handles System tick timer.
 */
UTOS_EXPORT void SysTick_Handler(void) {
  HAL_IncTick();

  if (utos_current_task)
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // Trigger PendSV interrupt
}
