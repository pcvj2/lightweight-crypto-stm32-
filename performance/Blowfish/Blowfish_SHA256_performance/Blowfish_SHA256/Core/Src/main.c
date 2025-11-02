/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Blowfish + Firmware-Integrity (SHA256) Performance Harness
  ******************************************************************************
  * This firmware combines Blowfish encryption and decryption cycles with a firmware
  * integrity check using SHA-256 hashing. It is for deployment on STM32F7 board.
  *
  *
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "com.h"           // COM_Init(), printf() wiring
#include "hashcheck.h"     // FW_Hash_Verify()
#include "blowfish.h"      // Blowfish_Init, Blowfish_Encrypt/Decrypt
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef   hcrc;
UART_HandleTypeDef  huart3;
TIM_HandleTypeDef   htim11;

/* USER CODE BEGIN PV */
RNG_HandleTypeDef  hrng;

// Blowfish 128-bit key
static const uint32_t BF_KEY[4] = {
  0xABCDEFAB, 0xCDEFABCD,
  0xEFABCDEF, 0xABCDEFAB
};
// our test‐block
static const uint32_t BF_PLAIN[2] = { // 64-bit plaintext
  0x12300325, 0x89646238
};

uint32_t bf_ct[2], bf_dt[2]; // Ciphertext and decrypted buffers

// SHA-256 digest buffer
uint8_t  sha_digest[32];
int32_t  sha_len;
/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM11_Init(void);

/* USER CODE BEGIN 0 */
#include "stm32f7xx_ll_cortex.h"


uint32_t getCurrentMicros(void); // Microsecond timer using SysTick functions
/* USER CODE END 0 */


/**
  * @brief  The application entry point.
  */
int main(void)
{
  uint32_t start, stop, elapsed;
  char     uart_buf[80];
  int      uart_len;
  BLOWFISH_CTX bf_ctx;

  /* MCU init */
  HAL_Init();
  SystemClock_Config();

  /* Peripherals init */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_CRC_Init();
  MX_TIM11_Init();

  // COM setup and firmware integrity check
  COM_Init();
  printf("\r\n=== COM Initialized ===\r\n");
  printf(">>> Verifying FW SHA-256...\r\n");
  FW_Hash_Verify();    // loops forever on failure
  printf(">>> FW Hash OK — proceeding to Blowfish+SHA benchmark\r\n");

  // One‐off Blowfish test
  memcpy(&bf_ctx, &(BLOWFISH_CTX){0}, sizeof(bf_ctx));
  Blowfish_Init(&bf_ctx, (uint8_t*)BF_KEY, sizeof(BF_KEY));

  memcpy(bf_ct, BF_PLAIN, sizeof(bf_ct));
  Blowfish_Encrypt(&bf_ctx, &bf_ct[0], &bf_ct[1]);

  memcpy(bf_dt, bf_ct, sizeof(bf_dt));
  Blowfish_Decrypt(&bf_ctx, &bf_dt[0], &bf_dt[1]);

  printf("BF test CT: {%08lx %08lx}\r\n",
         (unsigned long)bf_ct[1], (unsigned long)bf_ct[0]);

  printf("BF test PT: {%08lx %08lx}\r\n",
         (unsigned long)bf_dt[1], (unsigned long)bf_dt[0]);
  HAL_Delay(500);

  /* Benchmark 100 runs of (Blowfish + SHA-256) */
  HAL_TIM_Base_Start(&htim11);
  while (1)
  {
      start = getCurrentMicros();

      for (int i = 0; i < 100; i++) {
        // reset plaintext in place
        memcpy(bf_ct, BF_PLAIN, sizeof(bf_ct));
        // encrypt/decrypt
        Blowfish_Encrypt(&bf_ctx, &bf_ct[0], &bf_ct[1]);
        Blowfish_Decrypt(&bf_ctx, &bf_ct[0], &bf_ct[1]);

        // run SHA-256 on the ORIGINAL PLAIN text
        STM32_SHA256_HASH_DigestCompute(
          (uint8_t*)BF_PLAIN,
          sizeof(BF_PLAIN),
          sha_digest,
          &sha_len
        );
      }

      stop = getCurrentMicros();
      elapsed = stop - start;

      uart_len = snprintf(uart_buf, sizeof(uart_buf),
        "BF+SHA256 (100 runs): %4lu us\r\n",
        (unsigned long)elapsed
      );
      HAL_UART_Transmit(&huart3, (uint8_t*)uart_buf, uart_len, HAL_MAX_DELAY);
      HAL_Delay(1000);
  }
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */


/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 95;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 65535;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED3_Pin */
  GPIO_InitStruct.Pin = LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED3_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */

/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// Microsecond function for timing using SysTick
uint32_t getCurrentMicros(void)
{
    LL_SYSTICK_IsActiveCounterFlag();  // Clear pending flag
    uint32_t m = HAL_GetTick();
    const uint32_t tms = SysTick->LOAD + 1;
    __IO uint32_t u = tms - SysTick->VAL;
    if (LL_SYSTICK_IsActiveCounterFlag()) {
        m = HAL_GetTick();
        u = tms - SysTick->VAL;
    }
    return (m * 1000 + (u * 1000) / tms);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
