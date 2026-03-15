#include "main.h"
#include "stm32f0xx_hal.h"

void SystemClock_Config(void);
void init_adc(void) {
    // 1. Enable ADC1 in RCC [cite: 616]
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN; // Enable GPIOC for PC0

    // 2. Configure PC0 as Analog mode [cite: 613]
    GPIOC->MODER |= (3 << (0 * 2)); // PC0 to Analog mode

    // 3. Configure ADC: 8-bit resolution, continuous mode, software trigger [cite: 616]
    ADC1->CFGR1 |= ADC_CFGR1_CONT; // Continuous conversion mode
    ADC1->CFGR1 |= (2 << ADC_CFGR1_RES_Pos); // 8-bit resolution

    // 4. Select Channel 10 (for PC0) [cite: 565, 597, 617]
    ADC1->CHSELR |= ADC_CHSELR_CHSEL10;

    // 5. Self-calibration [cite: 617]
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL); // Wait for calibration to finish [cite: 575]

    // 6. Enable and Start ADC [cite: 617]
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)); // Wait until ADC is ready [cite: 577]
    ADC1->CR |= ADC_CR_ADSTART; // Start conversion [cite: 577]
}

void init_leds(void) {
    // Enable GPIO for LEDs (assuming standard Discovery board pins, e.g., GPIOC pins 6-9)
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER |= (1 << (6 * 2)) | (1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2));
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  init_leds();
  init_adc();
  while (1) {
      // Read ADC data register (8-bit value: 0-255) [cite: 566, 620]
      uint8_t val = ADC1->DR;

      // Turn on/off LEDs based on 4 increasing thresholds [cite: 621, 622]
      if (val > 50)  GPIOC->BSRR = (1 << 7);  else GPIOC->BSRR = (1 << (7 + 16));
      if (val > 100) GPIOC->BSRR = (1 << 8);  else GPIOC->BSRR = (1 << (8 + 16));
      if (val > 150) GPIOC->BSRR = (1 << 6);  else GPIOC->BSRR = (1 << (6 + 16));
      if (val > 200) GPIOC->BSRR = (1 << 9);  else GPIOC->BSRR = (1 << (9 + 16));
  }
  return -1;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add their own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add their own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
