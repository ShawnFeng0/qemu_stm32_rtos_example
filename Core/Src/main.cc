#include <stdint.h>
#include <stdio.h>

#include "debug_log.h"
#include "event.h"
#include "stm32f4xx_hal.h"
#include "task.h"

void SystemClock_Config(void);
static void Error_Handler(void);

#define DEFINE_TOGGLE_TASK(name, interval_ms, led)         \
  void *name(void *param) {                                \
    LOGGER_DEBUG("%s: param: %u", #name, (unsigned)param); \
    auto timestamp = HAL_GetTick();                        \
    while (true) {                                         \
      if (HAL_GetTick() - timestamp > (interval_ms)) {     \
        timestamp = HAL_GetTick();                         \
        LOGGER_DEBUG("%s: heartbeat", #name);              \
      }                                                    \
    }                                                      \
  }

DEFINE_TOGGLE_TASK(task0, 512, LED3)
DEFINE_TOGGLE_TASK(task1, 1024, LED4)
DEFINE_TOGGLE_TASK(task2, 2048, LED5)
DEFINE_TOGGLE_TASK(task3, 4096, LED6)

utos::Event event;
[[noreturn]] void *task_notify(void *param) {
  LOGGER_DEBUG("param: %u", (unsigned)param);
  auto timestamp = HAL_GetTick();
  while (true) {
    if (HAL_GetTick() - timestamp > 1000) {
      timestamp = HAL_GetTick();
      LOGGER_DEBUG("post event...");
      event.notify_all();
    }
  }
}

[[noreturn]] void *task_wait(void *param) {
  LOGGER_DEBUG("param: %u", (unsigned)param);
  while (true) {
    event.wait(utos_current_task, 1000);
    LOGGER_DEBUG("received event.");
  }
}

[[noreturn]] void *task_wait1(void *param) {
  LOGGER_DEBUG("param: %u", (unsigned)param);
  while (true) {
    event.wait(utos_current_task, 1000);
    LOGGER_DEBUG("received event.");
  }
}

int main(void) {
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
  HAL_Init();

  LOGGER_INFO("System starting...");

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();

  //  task_init(task0, (void *)10, 0, 2048);
  //  task_init(task1, (void *)11, 1, 2048);
  //  task_init(task2, (void *)12, 2, 2048);
  //  task_init(task3, (void *)13, 3, 2048);
  task_init(task_notify, (void *)13, 3, 2048);
  task_init(task_wait, (void *)13, 3, 2048);
  task_init(task_wait1, (void *)13, 3, 2048);

  start_scheduler();

  /* Infinite loop */
  while (1) {
  }
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
