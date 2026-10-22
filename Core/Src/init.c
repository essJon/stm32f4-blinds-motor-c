/**
  ******************************************************************************
  * @file           : init.c
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Init file for peripherals
  *
  * This file contains all the init functions for serial input, gpio button, gpio
  * LEDs, RTC, TIM6 and TIM9, PWM, ADC and I2C. Additionally, all interrupts for
  * the program are at the bottom of the file.
  *
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

#include <stdio.h>

#include "init.h"
#include "main.h"

/* Global variables ----------------------------------------------------------*/
static UART_HandleTypeDef serial_io;
RTC_HandleTypeDef hrtc;
TIM_HandleTypeDef  g_tim9;
ADC_HandleTypeDef g_adc_in;
I2C_HandleTypeDef g_isc1;
TIM_HandleTypeDef g_tim6;

unsigned char async_buff[BUF_SIZ];

unsigned int g_count;
unsigned int g_accu;
int scene_alarm_flag;

volatile unsigned char g_flag_pend_bus;
volatile unsigned char g_flag_recv_bus;
volatile unsigned char g_flag_errr_bus;

int tim6_period_elapsed;

int error_state_flag = 0;
ErrorCause error_cause = NO_ERROR;

/**
 * @brief Initialisation function for USART 6 Serial communication
 *
 * Initialises PORT C of GPIO and sets settings fro serial communication such as baudrate,
 * word length, stop bits, parity, and sets the pins used for serial IO
 *
 * @retval None
 */
void init_serial_io(){
	GPIO_InitTypeDef gpio_init;

	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_USART6_CLK_ENABLE();

	gpio_init.Pin = GPIO_PIN_6 |GPIO_PIN_7;
	gpio_init.Mode = GPIO_MODE_AF_PP;
	gpio_init.Pull = GPIO_PULLUP;
	gpio_init.Speed = GPIO_SPEED_FAST;
	gpio_init.Alternate = GPIO_AF8_USART6;

	HAL_GPIO_Init(GPIOC, &gpio_init);

    serial_io.Instance = USART6;
    serial_io.Init.BaudRate = 115200;
    serial_io.Init.WordLength = UART_WORDLENGTH_8B;
    serial_io.Init.StopBits = UART_STOPBITS_1;
    serial_io.Init.Parity = UART_PARITY_NONE;
    serial_io.Init.Mode = UART_MODE_TX_RX;
    serial_io.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    serial_io.Init.OverSampling = UART_OVERSAMPLING_16;
	if(HAL_UART_Init(&serial_io) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief Implementaition of the weak function io_getchar();
 *
 * @retval ch Returns the ascii code of the recieved letter
 */
int __io_getchar(void)
{
	/* Local Variable */
	int ch = 0;

	/* Clear the Overrun flag just before receiving the first character */
	__HAL_UART_CLEAR_OREFLAG(&serial_io);

	/* Wait for reception of a character on the USART RX line and echo this
	 * character on console */
	HAL_UART_Receive(&serial_io, (uint8_t *)&ch, 1, HAL_MAX_DELAY);

	if(ch == '\r')
	{
		HAL_UART_Transmit(&serial_io, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
		ch = '\n';
	}

	HAL_UART_Transmit(&serial_io, (uint8_t *)&ch, 1, HAL_MAX_DELAY);

	return ch;
}

/**
 * @brief Implementaition of the weak function io_putchar();
 *
 * @retval ch Returns the ascii code of the recieved letter
 */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&serial_io, (uint8_t *)&ch, 1, HAL_MAX_DELAY);

	return ch;
}

/**
 * @brief Initialisation of the GPIO button
 *
 * Function enables the clock for PORT A where the user button is and
 * configures PIN 0 which is the blue user button
 *
 * @retval Success Value 0 = HAL Success
 */
int init_gpio_btn(void){
	/* Local Variables */
	GPIO_InitTypeDef gpio_conf = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin : PA0 */
	gpio_conf.Pin = GPIO_PIN_0;
	gpio_conf.Mode = GPIO_MODE_INPUT;
	gpio_conf.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_conf);

	return 0;
}

/**
 * @brief Initialisation of the GPIO LED
 *
 * Function enables the clock for PORT D where the LEDs are located
 * and configures PIN 12,13,14,15 which match the outputs for the LEDs
 *
 * @retval Success Value 0 = HAL Success
 */
int init_gpio_LED(void){
	/* Local Variables */
	GPIO_InitTypeDef gpio_conf = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pins : PD12-15 */
	gpio_conf.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio_conf.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_conf.Pull = GPIO_NOPULL;
	gpio_conf.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &gpio_conf);

	return 0;
}

/**
 * @brief Initialisation of the Real Time Clock (RTC)
 *
 * Function enables RTC in 24hr format, with predivision values ensuring clock ticks once per
 * second. This function also sets a default time and date of 1/1/2000 at 00:00:00
 *
 * @retval Success Value 0 = HAL Success
 *
 * @see get_user_scene_times()
 */
int init_rtc(void)
{
	/* Local Variables */
	RTC_TimeTypeDef s_time =    {0};
	RTC_DateTypeDef s_date =    {0};

	/* Configure properties of RTC */
	hrtc.Instance = 			RTC;
	hrtc.Init.HourFormat =  	RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv =    124;
	hrtc.Init.SynchPrediv =     7999;
	hrtc.Init.OutPut =          RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity =  RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType =      RTC_OUTPUT_TYPE_OPENDRAIN;

	if(HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}

	/* Set RTC Date/Time - BCD Format */
	s_date.WeekDay =   RTC_WEEKDAY_MONDAY;
	s_date.Month =     RTC_MONTH_JANUARY;
	s_date.Date =      0x01;
	s_date.Year =      0x00;

	HAL_RTC_SetDate(&hrtc, &s_date, RTC_FORMAT_BCD);

	s_time.Hours =    0x00;
	s_time.Minutes =  0x00;
	s_time.Seconds =  0x00;

	HAL_RTC_SetTime(&hrtc, &s_time, RTC_FORMAT_BCD);

	return 0;
}

/**
 * @brief Initialisation of Timer 9 and Pulse Width Modulation
 *
 * Function enables Timer 9 to generate a frame of 20ms (50Hz) using prescaler value of 399 and period of 1599
 * Then PWM is enabled on channel 1 (GPIOE PIN PE5) with duty cycle configurable in the CCR register
 * Timer 9 runs at 32Mhz from APB2
 * 1 ms is 80 ticks of the clock
 *
 * @retval Success Value 0 = HAL Success
 */
int init_pwm(void)
{
	/* Local Variables */
	TIM_ClockConfigTypeDef clk_src_cfg = {0};
	TIM_OC_InitTypeDef oc_cfg = {0};
	TIM_MasterConfigTypeDef tim_mstr_cfg = {0};

	g_tim9.Instance = TIM9;
	g_tim9.Init.Prescaler = (400) - 1;
	g_tim9.Init.CounterMode = TIM_COUNTERMODE_UP;
	g_tim9.Init.Period = (1600) - 1;
	g_tim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	g_tim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	if(HAL_TIM_Base_Init(&g_tim9) != HAL_OK)
		{
			Error_Handler();
		}

	clk_src_cfg.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&g_tim9, &clk_src_cfg);

	oc_cfg.OCMode = TIM_OCMODE_PWM1;
	oc_cfg.Pulse = (15) - 1;
	oc_cfg.OCPolarity = TIM_OCPOLARITY_HIGH;
	oc_cfg.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&g_tim9, &oc_cfg, TIM_CHANNEL_1);

	tim_mstr_cfg.MasterOutputTrigger = TIM_TRGO_RESET;
	tim_mstr_cfg.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	if(HAL_TIMEx_MasterConfigSynchronization(&g_tim9, &tim_mstr_cfg) != HAL_OK)
	{
		Error_Handler();
	}

	if(HAL_TIM_PWM_Init(&g_tim9) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_TIM_MspPostInit(&g_tim9);

	return 0;
}

/**
 * @brief Initialisation of ADC
 *
 * Configures ADC1 to receive analogue data from Pin PA1
 *
 * @retval Success Value 0 = HAL Success
 */
int init_gpio_adc(void)
{
	/* Local Variables */
	ADC_ChannelConfTypeDef adc_ch_conf;
	GPIO_InitTypeDef gpio_conf = {0};

	/*Configure GPIO pin : PA1 */
	gpio_conf.Pin = GPIO_PIN_1;
	gpio_conf.Mode = GPIO_MODE_ANALOG;
	gpio_conf.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_conf);

	g_adc_in.Instance = ADC1;
	g_adc_in.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
	g_adc_in.Init.Resolution = ADC_RESOLUTION_12B;
	g_adc_in.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	g_adc_in.Init.ScanConvMode = DISABLE;
	g_adc_in.Init.ContinuousConvMode = ENABLE;
	g_adc_in.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	g_adc_in.Init.NbrOfConversion = 1;
	g_adc_in.Init.DMAContinuousRequests = DISABLE;
	HAL_ADC_Init(&g_adc_in);

	adc_ch_conf.Channel = ADC_CHANNEL_1;
	adc_ch_conf.Rank = 1;
	adc_ch_conf.SamplingTime = ADC_SAMPLETIME_480CYCLES;

	if(HAL_ADC_ConfigChannel(&g_adc_in, &adc_ch_conf) != HAL_OK)
	{
		Error_Handler();
	}

	return 0;
}

/**
 * @brief Initialisation of I2C
 *
 * Configures I2C so that program can receive data from a target device
 * Sets addressing mode to 7-bit
 *
 * @retval None
 */
void init_i2c(void)
{
	g_isc1.Instance = I2C1;
	g_isc1.Init.ClockSpeed = 400000;
	g_isc1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	g_isc1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	g_isc1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	/* Additional configuration that may be useful for a Target or Multi-Controller setup */
	g_isc1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	g_isc1.Init.OwnAddress1 = 0x01 << 1;

	if(HAL_I2C_Init(&g_isc1) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief Initialisation of Timer 6
 *
 * Configures Timer 6 to a period of 20ms with prescaler values of 83 and period of 19999
 * Timer 6 is used to recieve data from I2C at a set interval
 * Timer 6 runs at 64Mhz on APB1
 *
 * @retval None
 */
void init_tim6(void)
{
	g_tim6.Instance = TIM6;
	g_tim6.Init.Prescaler = (84 - 1);
	g_tim6.Init.Period = (20000) - 1;

	if(HAL_TIM_Base_Init(&g_tim6) != HAL_OK)
	{
		Error_Handler();
	}
}

/***Interrupts and Callbacks**************************************************************************************************/

/**
 * @brief This interupt is called when ADC values are received
 *
 * @retval None
*/
void ADC_IRQHandler(void)
{
	HAL_ADC_IRQHandler(&g_adc_in);
}

/**
 * @brief Oversampling callback function, that accumulates ADC Values and collects count for averaging later
 * @param hadc Struct that holds ADC configuration
 *
 * @retval None
 *
 * @see process_adc()
*/
void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef* hadc)
{
	g_accu += HAL_ADC_GetValue(&g_adc_in);
	g_count++;
}

/**
 * @brief Interrupt that occurs when one of the alarms triggers
 *
 * Then the specific alarm that triggered the interrupt has its callback called
 *
 * @retval None
*/
void RTC_Alarm_IRQHandler(void)
{
	HAL_RTC_AlarmIRQHandler(&hrtc);
}

/**
 * @brief Alarm A event callback function
 *
 * Sets the scene_alarm_flag to 0, so the scene state will open the blinds
 *
 * @param rtcInstance HAL required but unused
 * @retval None
*/
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *rtcInstance)
{
	scene_alarm_flag = 0;
}

/**
 * @brief Alarm B event callback function
 *
 * Sets the scene_alarm_flag to 1, so the scene state will set the blind position to user set target angle
 *
 * @param rtcInstance HAL required but unused
 * @retval None
*/
void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *rtcInstance)
{
	scene_alarm_flag = 1;
}

/**
* @brief This function handles I2C1 event interrupt
* @retval None
*/
void I2C1_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&g_isc1);
}

/**
 * @brief This function handles I2C1 error interrupt.
 * @retval None
*/
void I2C1_ER_IRQHandler(void)
{
	HAL_I2C_ER_IRQHandler(&g_isc1);
}

/**
 * @brief This function handles Timer 6 interrupt.
 * @retval None
*/
void TIM6_DAC_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&g_tim6);
}

/**
 * @brief Error callback that sets the I2C error flag to 1
 * @param hndI2C Struct containing I2C config. Unused
 * @retval None
 *
 * @see process_i2c()
*/
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hndI2C)
{
	g_flag_errr_bus = 1;
}

/**
 * @brief I2C data received callback that sets the received flag to 1
 * @param hndI2C Struct containing I2C config. Unused
 * @retval None
 *
 * @see process_i2c()
*/
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hndI2C)
{
	g_flag_recv_bus = 1;
}

/**
 * @brief Timer period elapsed callback
 *
 * If timer 6 has elapsed, set the timer 6 elapsed flag to 1
 *
 * @param h_triggered_timer Struct containing timer configuration, in this case used to identify timer
 * @retval None
 *
 * @see process_i2c()
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* h_triggered_timer)
{
	/* Filter for Timer */
	if(h_triggered_timer == &g_tim6)
	{
		tim6_period_elapsed = 1;
	}
}
