/**
  ******************************************************************************
  * @file           : scene_operation_state.c
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Scene operation state implementation
  *
  * This file holds the implementation for the Scene operation state. This
  * state changes the position of the motor and blinds based on the user
  * configured schedules and target angle. The motor will move between open
  * and the user configured angle at the time specified
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
#include "init.h"
#include "util.h"

/**
 * @brief Scene state function that responds to alarms to move motor
 *
 * Alarms are triggered as interrupts, alarm A will open the blinds, while
 * alarm B will move the motor to a user defined position
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval Next state to transition to in state machine
 *
 * @see init_rtc()
 * @see get_user_scene_times()
 * @see HAL_RTC_AlarmAEventCallback()
 * @see HAL_RTCEx_AlarmBEventCallback()
 */
State state_scene_operation(MachineData *ptr_data)
{
	State ret_val = STATE_SCENE_OPERATION;

	if(!ptr_data->config_flag)
	{
		printf("\n\n---------ENTERED SCENE OPERATION STATE---------\r\n");
		printf("\n----------------------------------------------------\r\n\n");
		ptr_data->config_flag = STATE_CONFIG_COMPLETE;
	}

	if (scene_alarm_flag){
		ptr_data->pwm_output_ms = calculate_pulse_ms(ptr_data->scene_target_angle);
	} else {
		ptr_data->pwm_output_ms = BLINDS_OPEN_MS;
	}

	if(HAL_GetTick() - ptr_data->last_tick >= SCENE_STATE_LED_ROTATION_MS)
	{
		GPIOD->ODR ^= 1 << GREENLED;
		ptr_data->last_tick = HAL_GetTick();
	}

	if(HAL_GetTick() - ptr_data->last_tick_print >= LIVE_DATA_PRINT_MS)
	{
		printf("\nCURRENT BLIND POSTION ANGLE:                        %d\r\n", calculate_blind_angle(ptr_data->pwm_output_ms));

		ptr_data->last_tick_print = HAL_GetTick();
	}

	if(ptr_data->bool_rising_edge)
	{
		GPIOD->ODR = RESET_LED;
		ret_val = STATE_AUTONOMOUS_OPERATION;
		ptr_data->config_flag = STATE_CONFIG_INCOMPLETE;
	}

	return ret_val;
}
