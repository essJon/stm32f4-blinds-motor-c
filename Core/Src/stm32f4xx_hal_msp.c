/**
  ******************************************************************************
  * @file         stm32f4xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  *
  *               The file configures pins and clocks used by peripherals
  *               in one centralised location, rather than spread across
  *               the main file. The main file is then used to configure
  *               attributes of the peripheral, rather than what the
  *               peripheral is routed to/from.
  *
  *               The HAL_MspInit provides a concrete impl. for the __weak
  *               function that is declared in the HAL drivers, therefore
  *               it is invoked when HAL_MspInit is called. That call is as
  *               a result of the HAL_Init call in the main function, which is
  *               a call to the driver where our concrete definition is used
  *               in lieu of the __weak function.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Steven Wheeldon
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* External functions --------------------------------------------------------*/
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
}


/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(hadc->Instance==ADC1)
  {
     /* Peripheral clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA1     ------> ADC1_IN1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }

}


/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA1     ------> ADC1_IN1
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
  }

}


/**
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(hi2c->Instance==I2C1)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  }

}


/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
  }

}


/**
* @brief RNG MSP Initialization
* This function configures the hardware resources used in this example
* @param hrng: RNG handle pointer
* @retval None
*/
void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng)
{
  if(hrng->Instance==RNG)
  {
    /* Peripheral clock enable */
    __HAL_RCC_RNG_CLK_ENABLE();
  }
}


/**
* @brief RNG MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrng: RNG handle pointer
* @retval None
*/
void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng)
{
  if(hrng->Instance==RNG)
  {
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();
  }

}


/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	/* Local Variables */
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(htim_base->Instance==TIM3)
	{
		/* Peripheral clock enable */
		__HAL_RCC_TIM3_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/**TIM3 GPIO Configuration
		PA6     ------> TIM3_CH1
		*/
		GPIO_InitStruct.Pin = 				GPIO_PIN_6;
		GPIO_InitStruct.Mode = 				GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = 				GPIO_NOPULL;
		GPIO_InitStruct.Speed = 			GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = 		GPIO_AF2_TIM3;

		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	else if(htim_base->Instance==TIM9)
	{
	  /* See note in HAL_TIM_MspPostInit */
	  __HAL_RCC_TIM9_CLK_ENABLE();
	}
	else if(htim_base->Instance==TIM6)
	{
	  /* Peripheral clock enable */
	  __HAL_RCC_TIM6_CLK_ENABLE();
	}
}


/* NOTE *
 * Manually swapped to TIM9 from TIM4 because of LED pin.
 *
 * TIM9 runs on APB2 at a different clock rate,
 * ensure that correct clock is used in user code.
 *
 * AF3 from Table 9 https://www.st.com/resource/en/datasheet/stm32f405rg.pdf
 * using PE5 for CH1 PWM.
 *
 * Luke 💡
 */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
	/* Local Variables */
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(htim->Instance==TIM9)
	{
		__HAL_RCC_GPIOE_CLK_ENABLE();

    	GPIO_InitStruct.Pin = 			GPIO_PIN_5;
    	GPIO_InitStruct.Mode = 			GPIO_MODE_AF_PP;
    	GPIO_InitStruct.Pull = 			GPIO_NOPULL;
    	GPIO_InitStruct.Speed = 		GPIO_SPEED_FREQ_LOW;
    	GPIO_InitStruct.Alternate = 	GPIO_AF3_TIM9;

    	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	}
}


/**
* @brief TIM_Base MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
	if(htim_base->Instance==TIM3)
	{
		/* Peripheral clock disable */
		__HAL_RCC_TIM3_CLK_DISABLE();

		/**TIM3 GPIO Configuration
		 * PA6     ------> TIM3_CH1
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
	}
	else if(htim_base->Instance==TIM9)
	{
		/* See note in HAL_TIM_MspPostInit */
		__HAL_RCC_TIM9_CLK_DISABLE();
	}
	else if(htim_base->Instance==TIM6)
	{
		/* Peripheral clock disable */
		__HAL_RCC_TIM6_CLK_DISABLE();
	}
}


/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(huart->Instance==USART6)
  {
    /* Peripheral clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }

}


/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==USART6)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART6_CLK_DISABLE();

    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);
  }
}


/**
* @brief RTC MSP Initialization
* This function configures the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
* 
* There is no populated LSE on the STM32F407 Disc1 therefore
* configured to use the LSI.
* Luke 💡
*/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if(hrtc->Instance==RTC)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = /*RCC_RTCCLKSOURCE_LSI*/ RCC_RTCCLKSOURCE_HSE_DIV8; /* Commented out LSI | Choose HSE clock and lose wake up and incur greater power consumption */
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
  }
}

/**
* @brief RTC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  }
}
