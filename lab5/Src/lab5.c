#include "main.h"
#include "stm32f0xx_hal.h"

void SystemClock_Config(void);

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

  while (1)
  {
 
  }
  return -1;
}
void GPIO_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOB->MODER &= ~(3<<(11*2));
    GPIOB->MODER &= ~(3<<(13*2));
    GPIOB->MODER |=  (2<<(11*2));
    GPIOB->MODER |=  (2<<(13*2));
    GPIOB->OTYPER |= (1<<11);
    GPIOB->OTYPER |= (1<<13);
    GPIOB->AFR[1] &= ~(0xF<<12); 
    GPIOB->AFR[1] |=  (1<<12);   // PB11
    GPIOB->AFR[1] &= ~(0xF<<20);
    GPIOB->AFR[1] |=  (1<<20);   // PB13
    GPIOB->MODER |= (1<<(14*2));
    GPIOB->BSRR = (1<<14);
    GPIOC->MODER |= (1<<(0*2));
    GPIOC->BSRR = (1<<0);
}
void I2C2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    I2C2->CR1 &= ~I2C_CR1_PE;
    I2C2->TIMINGR = 0x2000090E;
    I2C2->CR1 |= I2C_CR1_PE;
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
