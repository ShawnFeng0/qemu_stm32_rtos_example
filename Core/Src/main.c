#include "stm32f4_discovery.h"
#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stdio.h>

#define LOGGER_DEBUG(fmt, ...)                                                 \
  ({                                                                           \
    char buffer[256];                                                          \
    snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__);                      \
    PrintDebugStr(buffer);                                                     \
  })

static void PrintDebugStr(const char *str) {
  if (!str)
    return;

  while (*str) {
    ITM_SendChar(*str++);
  }
}

void SystemClock_Config(void);
static void Error_Handler(void);

typedef void *(*task_entry_t)(void *);

uint32_t *task_stack_init(task_entry_t task_entry, void *parameters,
                          uint32_t *top_of_stack) {
  uint32_t *stack_top = top_of_stack - 16;

  *(stack_top + 15) = (uint32_t)0x01000000;    // xPSR register
  *(stack_top + 14) = (uint32_t)task_entry;    // PC register
  *(stack_top + 13) = (uint32_t)Error_Handler; // LR register
                                               // R12 R3 R2 R1
  *(stack_top + 8) = (uint32_t)parameters;     // R0

  return stack_top;
}

void task_init(uint32_t *task, task_entry_t task_entry, void *parameters,
               uint32_t *stack, uint32_t stack_size) {
  *task = (uint32_t)task_stack_init(task_entry, parameters, stack + stack_size);
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

uint32_t task0_stack[2048];
uint32_t task1_stack[2048];
uint32_t task2_stack[2048];
uint32_t task3_stack[2048];

uint32_t curr_task = 0;
uint32_t next_task = 1;
uint32_t PSP_array[4];

#define DEFINE_TOGGLE_TASK(name, interval_ms, led)                             \
  void *name(void *param) {                                                    \
    LOGGER_DEBUG("%s: param: %u", #name, (unsigned)param);                     \
    while (1) {                                                                \
      if (HAL_GetTick() & (interval_ms)) {                                     \
        BSP_LED_On(led);                                                       \
      } else {                                                                 \
        BSP_LED_Off(led);                                                      \
      }                                                                        \
    }                                                                          \
  }

DEFINE_TOGGLE_TASK(task0, 512, LED3)
DEFINE_TOGGLE_TASK(task1, 1024, LED4)
DEFINE_TOGGLE_TASK(task2, 2048, LED5)
DEFINE_TOGGLE_TASK(task3, 4096, LED6)

void StartScheduler();

int main(void) {
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();

  /* Configure LED3, LED4, LED5 and LED6 */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);

  /* Configure KEY Button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

  task_init(&PSP_array[0], task0, (void *)10, task0_stack,
            ARRAY_SIZE(task0_stack));
  task_init(&PSP_array[1], task1, (void *)11, task1_stack,
            ARRAY_SIZE(task1_stack));
  task_init(&PSP_array[2], task2, (void *)12, task2_stack,
            ARRAY_SIZE(task2_stack));
  task_init(&PSP_array[3], task3, (void *)13, task3_stack,
            ARRAY_SIZE(task3_stack));

  StartScheduler();

  /* Infinite loop */
  while (1) {
  }
}

#define ASM(...) __asm volatile(#__VA_ARGS__)

static void StartFirstTask(void) __attribute__((naked));
static void StartFirstTask(void) {
  /* Use the NVIC offset register to locate the stack. */
  ASM(ldr r0, = 0xE000ED08);
  ASM(ldr r0, [r0]);
  ASM(ldr r0, [r0]);

  /* Set the msp back to the start of the stack. */
  ASM(msr msp, r0);

  /* Clear the bit that indicates the FPU is in use */
  ASM(mov r0, #0);
  ASM(msr control, r0);

  /* Globally enable interrupts. */
  ASM(cpsie i);
  ASM(cpsie f);
  ASM(dsb);
  ASM(isb);

  /* System call to start first task. */
  ASM(svc 0);
  ASM(nop);
}

void StartScheduler() {
  curr_task = 0;
  NVIC_SetPriority(PendSV_IRQn, 0xff);
//  __set_CONTROL(0x3);
//  __ISB();
  StartFirstTask();
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) __attribute__((naked));
void SVC_Handler(void) {
  ASM(ldr r1, curr_task_svc);
  ASM(ldr r3, psp_array_svc);

  // Load next context
  ASM(ldr r4, next_task_svc);
  ASM(ldr r4, [r4]);
  ASM(str r4, [r1]);
  ASM(ldr r0, [ r3, r4, lsl #2 ]);
  ASM(ldmia r0 !, {r4 - r11});
  ASM(msr psp, r0);
  ASM(bx lr);

  // Define C data
  ASM(.align 4);
  ASM(curr_task_svc :.word curr_task);
  ASM(next_task_svc :.word next_task);
  ASM(psp_array_svc :.word PSP_array);
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) __attribute__((naked));
void PendSV_Handler(void) {
  // Save current context
  ASM(mrs r0, psp);            // get stack pointer
  ASM(stmdb r0 !, {r4 - r11}); // push to stack
  ASM(ldr r1, curr_task_);
  ASM(ldr r2, [r1]);
  ASM(ldr r3, psp_array_);
  ASM(str r0, [ r3, r2, lsl #2 ]); // store r0 to r3[r2 * 4]

  // Load next context
  ASM(ldr r4, next_task_);
  ASM(ldr r4, [r4]);
  ASM(str r4, [r1]);
  ASM(ldr r0, [ r3, r4, lsl #2 ]);
  ASM(ldmia r0 !, {r4 - r11});
  ASM(msr psp, r0);
  ASM(bx lr);

  // Define C data
  ASM(.align 4);
  ASM(curr_task_ :.word curr_task);
  ASM(next_task_ :.word next_task);
  ASM(psp_array_ :.word PSP_array);
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) {
  HAL_IncTick();

  next_task = (curr_task + 1) % 4;

  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Trigger PendSV interrupt
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
