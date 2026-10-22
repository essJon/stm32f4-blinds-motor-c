/**
  ******************************************************************************
  * @file           : manual_operation_state.c
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Manual operation state implementation
  *
  * This file holds the implementation for the manual operation state. This
  * state changes the position of the motor and blinds based on an external device
  * connected by I2C that sends percentage values between 0% and 100% in steps of
  * 25%.
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

#include "state_machine.h"
#include "util.h"
#include "init.h"



static int process_i2c(MachineData *ptr_data);

/**
 * @brief Manual state function that takes a value from 0% to 100% from a target I2C device and moves motor accordingly
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval Next state to transition to in state machine
 */
State state_manual_operation(MachineData *ptr_data)
{
	State ret_val = STATE_MANUAL_OPERATION;

	if(!ptr_data->config_flag)
	{
		printf("\n\n---------ENTERED MANUAL OPERATION STATE---------\r\n");
		printf("----------------------------------------------------\r\n\n");
		g_flag_pend_bus = 0;
		g_flag_recv_bus = 0;
		g_flag_errr_bus = 0;
		ptr_data->config_flag = STATE_CONFIG_COMPLETE;
	}

	if(process_i2c(ptr_data))
	{
		ptr_data->pwm_output_ms = calculate_pulse_ms(ptr_data->buff);
	}

	if(HAL_GetTick() - ptr_data->last_tick >= MANUAL_STATE_LED_ROTATION_MS)
	{
		GPIOD->ODR ^= 1 << BLUELED;
		ptr_data->last_tick = HAL_GetTick();
	}

	if(HAL_GetTick() - ptr_data->last_tick_print >= LIVE_DATA_PRINT_MS)
	{
		printf("\nCURRENT BLIND POSTION ANGLE:                        %d\r\n",calculate_blind_angle(ptr_data->pwm_output_ms));

		ptr_data->last_tick_print = HAL_GetTick();
	}

	if(ptr_data->bool_rising_edge)
	{
		GPIOD->ODR = RESET_LED;
		ptr_data->config_flag = STATE_CONFIG_INCOMPLETE;
		ret_val = STATE_SCENE_OPERATION;
	}

	return ret_val;
}


/**
 * @brief Function to evaluate if new I2C data is inputted, and pass it to the main state function.
 *
 * The function first requests a byte of data from a target I2C device at a set interval
 * Next if new data is detected, the input is displayed in terminal and the state function is informed
 * by changing a flag value. If an error has occurred an interrupt will trigger a flag, and this function
 * will inform the user about the error and enter error state if error is triggered 50 times.
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval Value to tell main state function if new data has been received 1 - New Data
 *
 * @see init_i2c()
 * @see HAL_I2C_ErrorCallback()
 * @see HAL_I2C_MasterRxCpltCallback()
 * @see HAL_TIM_PeriodElapsedCallback()
 */

int process_i2c(MachineData *ptr_data)
{
	int retval = 0;

	if(tim6_period_elapsed == 1 && !g_flag_pend_bus)
	{
		HAL_I2C_Master_Receive_IT(&g_isc1, (0x42 << 1), &ptr_data->buff, 1);

		/* Update Flag */
		g_flag_pend_bus = 1;
		tim6_period_elapsed = 0;
	}

	if(g_flag_recv_bus)
	{
		if(ptr_data->buff != I2C_NO_DATA)
		{
			printf("New data: %d%%\r\n", ptr_data->buff);
			ptr_data->buff = (unsigned char)(((float)ptr_data->buff / 100.0f) * 180.0f);
			retval = 1;
		}

		/* Reset Flags */
		g_flag_pend_bus = 0;
		g_flag_recv_bus = 0;

		ptr_data->i2c_error_count = 0;
	}

	if(g_flag_errr_bus)
	{
		ptr_data->err = HAL_I2C_GetError(&g_isc1);
		ptr_data->i2c_error_count++;
		fprintf(stderr, "Error: %#04x\r\n", ptr_data->err);

		/* Rudimentary reinitialise */
		HAL_I2C_DeInit(&g_isc1);
		HAL_I2C_Init(&g_isc1);

		if (ptr_data->i2c_error_count >= I2C_MAX_ERROR_COUNT)
		{
			error_state_flag = 1;
			error_cause = I2C_FAIL;
		}

		g_flag_pend_bus = 0;
		g_flag_errr_bus = 0;
	}

	return retval;
}
