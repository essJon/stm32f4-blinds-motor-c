/**
  ******************************************************************************
  * @file           : error_state.c
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Error state implementation
  *
  * This file is the state the state machine when an unrecoverable error occurs,
  * such as failed inits, or runtime disconnection of the motor, or I2C device.
  * This state takes no input, simply moves the blinds to the closed postion if
  * possible and then tells the user to reset the device to continue.
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


/**
 * @brief Error state function to inform user that an unrecoverable error has occurred
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval Error State return value to ensure error state is never left
 */
State state_error(MachineData *ptr_data)
{
	State ret_val = STATE_ERROR;
	error_state_flag = 0;

	if(!ptr_data->config_flag)
	{
		GPIOD->ODR = RESET_LED;
		ptr_data->pwm_output_ms = BLINDS_CLOSED_MS;
		printf("\n\n---------ENTERED ERROR STATE---------\r\n");
		printf("\n----------------------------------------------------\r\n\n");

		printf("CURRENT BLIND POSTION ANGLE:                        %d\r\n\n", calculate_blind_angle(ptr_data->pwm_output_ms));
		printf("ERROR ENCOUNTERD\r\n\n");

		switch(error_cause){
			case INIT_FAIL:
				printf("INITIALISATION OF PERHIPERALS FAILED\r\n");
				break;

			case I2C_FAIL:
				printf("I2C ENCOUNTERED TOO MANY ERRORS IN A ROW\r\n");
				break;

			default:
				printf("ERROR UNKNOWN\r\n");
				break;
		}
		printf("RESET MODULE TO CONTINUE\r\n");
		ptr_data->config_flag = STATE_CONFIG_COMPLETE;
	}

	if(HAL_GetTick() - ptr_data->last_tick >= ERROR_STATE_LED_ROTATION_MS)
	{
		GPIOD->ODR ^= 1 << REDLED;
		ptr_data->last_tick = HAL_GetTick();
	}

	return ret_val;
}
