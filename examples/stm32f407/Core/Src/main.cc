#include <stdint.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"
#include "utos/debug_log.h"
#include "utos/event.h"
#include "utos/task.h"

void SystemClock_Config(void);
static void Error_Handler(void);

utos::Event event;

[[noreturn]] void *task_notify(void *param) {
  LOGGER_DEBUG("param: %u", (unsigned)param);
  while (true) {
    utos_task_sleep(1000);
    LOGGER_DEBUG("post event...");
    event.notify_all();
  }
}

[[noreturn]] void *task_wait(void *param) {
  LOGGER_DEBUG("param: %u", (unsigned)param);
  while (true) {
    if (event.wait(-1)) {
      LOGGER_DEBUG("Permanently block reading");
    } else {
      LOGGER_ERROR("ERROR: Permanently block reading");
    }
  }
}

[[noreturn]] void *task_timeout(void *param) {
  LOGGER_DEBUG("param: %u", (unsigned)param);
  while (true) {
    if (event.wait(300)) {
      LOGGER_DEBUG("received event.");
    } else {
      LOGGER_DEBUG("received timeout.");
    }
  }
}

[[noreturn]] void *task_high_priority(void *param) {
  LOGGER_DEBUG("param: %u", (unsigned)param);
  while (true) {
    if (utos::time::get_time_ms() & 4096) {
      __NOP();
    } else {
      utos_task_sleep(10);
    }
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

  utos_task_create("task_notify", task_notify, (void *)13, utos_min_priority(),
                   2048);
  utos_task_create("task_wait", task_wait, (void *)13, utos_min_priority(),
                   2048);
  utos_task_create("task_timeout", task_timeout, (void *)13,
                   utos_min_priority(), 2048);
  utos_task_create("task_high_priority", task_high_priority, (void *)13,
                   utos_max_priority(), 2048);

  utos_start_scheduler();

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
