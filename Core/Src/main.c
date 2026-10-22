/**
  ******************************************************************************
  * @file           : main.c
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 09/03/2026
  * @brief          : Main program body
  *
  * This file holds the main function which simply starts all the peripherals,
  * interrupts and clocks required for the program. Logic for button presses,
  * and motor smoothing is included here. Mainly this file is for the state
  * machine implementation, and the program loop will continuously call the
  * current states function.
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


/* Definitions ---------------------------------------------------------------*/

/* Includes (Vendor Libraries, Standard Libraries, Personal Libraries) -------*/
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "init.h"
#include "state_machine.h"
#include "util.h"

/* Type Definitions ----------------------------------------------------------*/

/* Extern Function Prototypes ------------------------------------------------*/

/* Static Function Prototypes (Generated, Initialisation, Logic) -------------*/
static void SystemClock_Config(void);

static int program_loop(void);

/* Global Variables (Extern, Static) -----------------------------------------*/
static func_state_t* const state_table [5] =
{ state_initial, state_autonomous_operation, state_manual_operation, state_scene_operation, state_error };

/**
  * @brief The application entry point.
  *
  * The developer is required to provide additional
  * configuration for the necessary features that will be enabled
  * for this application. Only the generated configuration is provided.
  *
  * Peripherals should be after system clocks.
  *
  * Initialisations are followed by interrupt enabling. Function ends by returning
  * program loop to enter state machine architecture
  *
  * @retval program loop
  */
int main(void)
{
	/* Configure and Init */
	HAL_Init();
	SystemClock_Config();
	init_gpio_btn();
	init_gpio_LED();
	init_serial_io();
	init_rtc();
	init_pwm();
	init_gpio_adc();
	init_i2c();
	init_tim6();
	setvbuf(stdin, NULL, _IONBF, 0);

	HAL_NVIC_EnableIRQ(ADC_IRQn);
	HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

	HAL_TIM_Base_Start_IT(&g_tim6);

	return program_loop();
}


/**
  * @brief System Clock Configuration
  *
  * Configure clocks, LSE is not populated
  * on this board therefore LSI used for RTC
  * Disabled LSI to use HSE, see MSP.
  * PORT E and ADC1 clock also enabled here
  *
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_ADC1_CLK_ENABLE();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = 		RCC_OSCILLATORTYPE_HSE /*| RCC_OSCILLATORTYPE_LSI*/;
  RCC_OscInitStruct.HSEState = 				RCC_HSE_ON;
  /*RCC_OscInitStruct.LSIState = 				RCC_LSI_ON;*/
  RCC_OscInitStruct.PLL.PLLState = 			RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = 		RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 				4;
  RCC_OscInitStruct.PLL.PLLN = 				128;
  RCC_OscInitStruct.PLL.PLLP = 				RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 				6;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = 			RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = 			RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = 		RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = 		RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = 		RCC_HCLK_DIV8;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
}


/**
  * @brief  This function is executed in case of error occurrence.
  *
  * Sets error_state flag to 1, which when entering the program loop will ensure error state is entered
  * @retval None
  *
  * Skeleton Error_Handler provided by vendor (ST Microelectronics)
  */
void Error_Handler(void)
{
  __disable_irq();
  error_state_flag = 1;
  error_cause = INIT_FAIL;
  while (1)
  {
	 program_loop();
  }
}

/**
  * @brief Function to call state functions based on value cur_state
  *
  * Based on what state cur_state is referring to, it calls
  * functions which are stored as function pointers in array stateTable
  *
  * @param cur_state State enum, declaring which state to call next
  * @param ptr_data Pointer to shared persistent state machine struct
  * @retval Function pointer from state_table array
  */
State machine(State cur_state, MachineData* ptr_data)
{
	return state_table[cur_state](ptr_data);
}

/**
  * @brief Main state machine program loop
  *
  * Runs indefinitely calling the current state returned from the previous state called
  * Defines a struct to hold persistent state data, setting initial values
  * CCR is adjusted here to change pulse width to move blinds
  * Functions are called to check button presses and to move the motor smoothly
  * Additionally the timer for PWM is started, and an interrupt for ADC started
  *
  * @retval int - status code
  *
  * @see process_motor_smoothing()
  * @see debounced_button_poll()
  */
static int program_loop(void)
{
	/* Local Variables */
	MachineData data = {0, ORANGELED, 0, BLINDS_CLOSED_MS, BLINDS_CLOSED_MS, BLINDS_CLOSED_MS};
	State cur_state = STATE_INITIAL;
	unsigned int last_bounce_ms = 0;
	unsigned int pwm_tick = HAL_GetTick();


	/*Debouncing Algorithm Variables*/
	GPIO_PinState btn_press = GPIO_PIN_RESET;
	GPIO_PinState btn_stable = GPIO_PIN_RESET;
	GPIO_PinState btn_prevs = GPIO_PIN_RESET;

	HAL_TIM_PWM_Start(&g_tim9, TIM_CHANNEL_1);
    HAL_ADC_Start_IT(&g_adc_in);

    g_tim9.Instance->CCR1 = BLINDS_CLOSED_MS * CRR_TICKS_PER_MS;

	/* Program Loop */
	for(;;)
	{
		if(error_state_flag)
		{
			cur_state = STATE_ERROR;
			data.config_flag = STATE_CONFIG_INCOMPLETE;
		}

		process_motor_smoothing(&data, &pwm_tick);

		btn_press = debounced_button_poll(&last_bounce_ms, &btn_stable, &btn_press);

		if(btn_press && btn_press != btn_prevs)
		{
			data.bool_rising_edge = 1;
		}

		cur_state = machine(cur_state, &data);

		data.bool_rising_edge = 0;
		btn_prevs = btn_press;
	}

	return 0;
}
/***************************************************************************************************************/
