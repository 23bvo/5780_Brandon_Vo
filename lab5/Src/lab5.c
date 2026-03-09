#include "main.h"
#include "stm32f0xx_hal.h"
#include <stdbool.h>

#define GYRO_ADDR 0x69
#define WHOAMI_REG 0x0F

void GPIO_Init(void)
{
    // Enable GPIOB and GPIOC clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // PB11 and PB13 -> Alternate function
    GPIOB->MODER &= ~(3<<(11*2));
    GPIOB->MODER &= ~(3<<(13*2));
    GPIOB->MODER |=  (2<<(11*2));
    GPIOB->MODER |=  (2<<(13*2));

    // Open drain
    GPIOB->OTYPER |= (1<<11);
    GPIOB->OTYPER |= (1<<13);

    // Set alternate function (AF1 for I2C2)
    GPIOB->AFR[1] &= ~(0xF<<12); 
    GPIOB->AFR[1] |=  (1<<12);   // PB11

    GPIOB->AFR[1] &= ~(0xF<<20);
    GPIOB->AFR[1] |=  (1<<20);   // PB13

    // PB14 output high
    GPIOB->MODER |= (1<<(14*2));
    GPIOB->BSRR = (1<<14);

    // PC0 output high
    GPIOC->MODER |= (1<<(0*2));
    GPIOC->BSRR = (1<<0);
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
#define I2C_TIMEOUT 100000u

static bool I2C_WaitFlag(uint32_t flag)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (timeout--)
    {
        uint32_t isr = I2C2->ISR;
        if (isr & (I2C_ISR_NACKF | I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_OVR))
        {
            I2C2->ICR = I2C_ICR_NACKCF | I2C_ICR_ARLOCF | I2C_ICR_BERRCF | I2C_ICR_OVRCF;
            return false;
        }

        if (isr & flag)
            return true;
    }

    return false;
}

static void I2C_GenerateStop(void)
{
    I2C2->CR2 |= I2C_CR2_STOP;
    uint32_t timeout = 1000;
    while ((I2C2->ISR & I2C_ISR_STOPF) == 0 && timeout--);
    I2C2->ICR |= I2C_ICR_STOPCF;
}

static bool I2C_Read(uint8_t reg, uint8_t *out)
{
    /* Transmit register address (Write) */
    I2C2->CR2 = (GYRO_ADDR << 1) | (1 << 16);
    I2C2->CR2 &= ~I2C_CR2_RD_WRN;
    I2C2->CR2 |= I2C_CR2_START;

    if (!I2C_WaitFlag(I2C_ISR_TXIS))
    {
        I2C_GenerateStop();
        return false;
    }

    I2C2->TXDR = reg;

    if (!I2C_WaitFlag(I2C_ISR_TC))
    {
        I2C_GenerateStop();
        return false;
    }

    /* Read one byte */
    I2C2->CR2 = (GYRO_ADDR << 1) | (1 << 16) | I2C_CR2_RD_WRN;
    I2C2->CR2 |= I2C_CR2_START;

    if (!I2C_WaitFlag(I2C_ISR_RXNE))
    {
        I2C_GenerateStop();
        return false;
    }

    *out = (uint8_t)I2C2->RXDR;

    I2C_GenerateStop();
    return true;
}

static bool I2C_Write(uint8_t reg, uint8_t value)
{
    I2C2->CR2 = (GYRO_ADDR << 1) | (2 << 16);
    I2C2->CR2 &= ~I2C_CR2_RD_WRN;
    I2C2->CR2 |= I2C_CR2_START;

    if (!I2C_WaitFlag(I2C_ISR_TXIS))
    {
        I2C_GenerateStop();
        return false;
    }
    I2C2->TXDR = reg;

    if (!I2C_WaitFlag(I2C_ISR_TXIS))
    {
        I2C_GenerateStop();
        return false;
    }
    I2C2->TXDR = value;

    if (!I2C_WaitFlag(I2C_ISR_TC))
    {
        I2C_GenerateStop();
        return false;
    }

    I2C_GenerateStop();
    return true;
}

uint8_t I2C_Read_WHOAMI()
{
    uint8_t data = 0;
    if (I2C_Read(WHOAMI_REG, &data))
        return data;

    return 0;
}

void Gyro_Init()
{
    /* PD bit = 1: normal mode */
    I2C_Write(0x20, 0x08);
}
int16_t read_axis(uint8_t low_reg)
{
    uint8_t low, high;

    if (!I2C_Read(low_reg, &low))
        return 0;
    if (!I2C_Read(low_reg + 1, &high))
        return 0;

    return (int16_t)((high << 8) | low);
}
void delay()
{
    for(volatile int i=0; i<1000000; i++);
}
#define THRESHOLD 2000

void update_leds(int16_t x, int16_t y)
{
    if(x > THRESHOLD)
        GPIOC->ODR |= (1<<8); // orange ON
    else if(x < -THRESHOLD)
        GPIOC->ODR |= (1<<7); // blue ON

    if(y > THRESHOLD)
        GPIOC->ODR |= (1<<9); // green ON
    else if(y < -THRESHOLD)
        GPIOC->ODR |= (1<<6); // red ON
}
int main()
{
    GPIO_Init();
    LED_Init();
    uint8_t id;
    int16_t x, y;
    id = I2C_Read_WHOAMI();

    if(id == 0xD3)
    {
        for(int i=0; i<5; i++)
        {
            GPIOC->ODR ^= (0xF << 9);
            delay();
        }
    }

    Gyro_Init();

    while(1)
    {
        // Read X and Y axes
        x = read_axis(0x28);
        y = read_axis(0x2A);
        // Update LEDs based on rotation direction
        update_leds(x, y);
        // Wait 
        delay();
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
