#include "main.h"
#include "stm32f0xx_hal.h"

char USART1_Read(void);
void SystemClock_Config(void);
int LED_Control(char c, char cmd);
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
  USART1_Init();
  LED_Init();
  while (1)
  {
    USART1_WriteString("CMD?\r\n");
    char led = USART1_Read();     // light
    char cmd = USART1_Read();     // what it does

    int result = LED_Control(led, cmd);
    if(result == 2)
    {
        USART1_Write(led);
        USART1_Write(cmd);
        USART1_WriteString("\r\n");
    }
  }
  return -1;
}

void USART1_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
    GPIOA->MODER |=  (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);
    GPIOA->AFR[1] &= ~((0xF << 4) | (0xF << 8));   
    GPIOA->AFR[1] |=  (1 << 4) | (1 << 8);         
    USART1->CR1 &= ~USART_CR1_UE;
    USART1->BRR = 8000000 / 9600;   // = 833
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;
}
void USART1_Write(char c)
{
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = c;
}
char USART1_Read(void)
{
    while (!(USART1->ISR & USART_ISR_RXNE));
    return USART1->RDR;
}
void USART1_WriteString(char *str)
{
    while (*str)
    {
        USART1_Write(*str++);
    }
}
void LED_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(GPIO_MODER_MODER6 |
                      GPIO_MODER_MODER7 |
                      GPIO_MODER_MODER8 |
                      GPIO_MODER_MODER9);
    GPIOC->MODER |= (GPIO_MODER_MODER6_0 |
                     GPIO_MODER_MODER7_0 |
                     GPIO_MODER_MODER8_0 |
                     GPIO_MODER_MODER9_0);
}
int LED_Control(char c, char cmd)
{
    int returnValue = 0;
    uint16_t pin = 0;
    switch(c)
    {
        case 'r':
            pin= GPIO_ODR_6;   // Red
            returnValue++;
            break;

        case 'b':
            pin= GPIO_ODR_7;   // blue
            returnValue++;
            break;

        case 'o':
            pin= GPIO_ODR_8;   // orange
            returnValue++;
            break;

        case 'g':
            pin= GPIO_ODR_9;   // Green
            returnValue++;
            break;
        default:
           USART1_WriteString("Invalid LED!\r\n");
            break;
    }
    switch(cmd)
    {
      case '0':   // OFF
            GPIOC->ODR &= ~pin;
            returnValue++;
            break;

        case '1':   // ON
            GPIOC->ODR |= pin;
            returnValue++;
            break;

        case '2':   // TOGGLE
            GPIOC->ODR ^= pin;
            returnValue++;
            break;

        default:
            USART1_WriteString("Invalid Command!\r\n");
            break;
    }
    return returnValue;
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
