/**
  ******************************************************************************
  * @file           : autonomous_operation_state.c
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Autonomous operation state implementation
  *
  * This file holds the implementation for the autonomous operation state. This
  * state changes the position of the motor and blinds based on a reading from
  * an LDR (Light Dependent Resistor).
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
#include <math.h>

#include "state_machine.h"
#include "init.h"
#include "util.h"

static void process_adc(MachineData *ptr_data);

/**
 * @brief Autonomous state function that moves servo motor based on ADC-derived light values
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval Next state to transition to in state machine
 */
State state_autonomous_operation(MachineData *ptr_data)
{
	State ret_val = STATE_AUTONOMOUS_OPERATION;

	if(!ptr_data->config_flag)
	{
		printf("\n\n---------ENTERED AUTONOMOUS OPERATION STATE---------\r\n");
		printf("\n----------------------------------------------------\r\n\n");
		ptr_data->config_flag = STATE_CONFIG_COMPLETE;
		ptr_data->max_adc_percent = 0;
		ptr_data->sum_adc_percent = 0;
		ptr_data->count_adc = 0;
		ptr_data->avg_adc_percent = 0;
	}

	if(HAL_GetTick() - ptr_data->last_tick >= AUTO_STATE_LED_ROTATION_MS)
	{

		if(++ptr_data->last_bit > BLUELED)
		{
			ptr_data->last_bit = GREENLED;
		}
		GPIOD->ODR = 1 << ptr_data->last_bit;

		ptr_data->last_tick = HAL_GetTick();
	}

	if(g_count >= 256)
	{
		process_adc(ptr_data);
	}

	if(HAL_GetTick() - ptr_data->last_tick_print >= LIVE_DATA_PRINT_MS)
	{
		printf("\n\nCURRENT BLIND POSTION ANGLE:                        %d\r\n", calculate_blind_angle(ptr_data->pwm_output_ms));
		printf("CURRENT LIGHT INTENSITY(V):                         %f\r\n", ptr_data->current_adc_mv / 1000.0f);
		printf("CURRENT LIGHT INTENSITY(%% OF CONFIGURED RANGE):    %d\r\n", ptr_data->current_adc_percent);
		printf("MAX RECORDED LIGHT INTENSITY SINCE ENTERED STATE:   %d\r\n", ptr_data->max_adc_percent);
		printf("AVG RECORDED LIGHT INTENSITY SINCE ENTERED STATE:   %d\r\n", ptr_data->avg_adc_percent);

		ptr_data->last_tick_print = HAL_GetTick();
	}

	if(ptr_data->bool_rising_edge)
	{
		GPIOD->ODR = RESET_LED;
		ret_val = STATE_MANUAL_OPERATION;
		ptr_data->config_flag = STATE_CONFIG_INCOMPLETE;
	}

	return ret_val;
}

/**
 * @brief Function to take raw LDR data and transform into usable percentages/time period
 *
 * First calculate average LDR mV after oversampling and invert, so bright is high mV.
 * Next it checks the value is not above or below configured thresholds, if so clamp to min/max percentage.
 * Otherwise convert to percentage.
 * Check if its a new max percentage, and calculate average percentage.
 * Finally convert percentage to angle, then to ms and pass to struct which later passes to motor movement
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 *
 * @see init_adc()
 */
static void process_adc(MachineData *ptr_data)
{
	unsigned int adc_val = g_accu / 256;
	float volts_per_bit = (3.3f/4095.0f);
	int adc_mv_range = 0;
	g_count = 0;
	g_accu = 0;
	int target_angle;

	/*Calculate V, then *1000 to get mV, round up to ceiling integer, then invert to get actual LDR mV and store in int machine data struct*/
	ptr_data->current_adc_mv = (int)(3300.0f - (ceil((adc_val * volts_per_bit) * 1000.0f)));

	if(ptr_data->current_adc_mv >= ptr_data->light_intensity_max_mv)
	{
		ptr_data->current_adc_percent = 100;
	}
	else if(ptr_data->current_adc_mv <= ptr_data->light_intensity_min_mv)
	{
		ptr_data->current_adc_percent = 0;
	} else {
		adc_mv_range = ptr_data->light_intensity_max_mv - ptr_data->light_intensity_min_mv;
		ptr_data->current_adc_percent = (int)ceil((((float)ptr_data->current_adc_mv - (float)ptr_data->light_intensity_min_mv) / (float)adc_mv_range) * 100.0f);
	}

	if (ptr_data->current_adc_percent > ptr_data->max_adc_percent)
	{
		ptr_data->max_adc_percent = ptr_data->current_adc_percent;
	}

	ptr_data->sum_adc_percent += ptr_data->current_adc_percent;
	ptr_data->count_adc++;

	ptr_data->avg_adc_percent = ptr_data->sum_adc_percent / ptr_data->count_adc;

	target_angle = (int)((ptr_data->current_adc_percent / 100.0f) * 180);

	ptr_data->pwm_output_ms = calculate_pulse_ms(target_angle);
}

