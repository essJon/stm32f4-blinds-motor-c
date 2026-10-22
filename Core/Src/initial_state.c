/**
  ******************************************************************************
  * @file           : initial_state.c
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/26
  * @brief          : Initial state implementation
  *
  * This file contains the implementations of the initial state. This state
  * prompts the user to enter values for LDR ranges, hysteresis bands, scene
  * state target angles, and times to flip between scene angle and open. These
  * values are then stored in the MachineData struct found in state_machine.h
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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "state_machine.h"
#include "init.h"
#include "util.h"

static int get_int(int *result_ptr ,int min_value, int max_value);
static int get_float(float *result_ptr ,float min_value, float max_value);

static void get_light_intensity_values(MachineData *ptr_data);
static void get_damping_period(MachineData *ptr_data);
static void get_hystereses_band_values(MachineData *ptr_data);
static void get_user_scene_target_angle(MachineData *ptr_data);
static void get_user_scene_times(MachineData *ptr_data, unsigned int alarm_id);


/**
 * @brief Initial state function that collects user configuration values for use later in the program
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval Next state to transition to in state machine
 */
State state_initial(MachineData *ptr_data)
{
	State ret_val = STATE_INITIAL;
	unsigned int alarm_a = RTC_ALARM_A;
	unsigned int alarm_b = RTC_ALARM_B;

	if(!ptr_data->config_flag)
	{
		GPIOD->ODR= 1 << ORANGELED;

		printf("\n----------------INITIAL SMART BLIND CONFIGURATION-----------------\n\r\n");
		printf("\n-----------------BLINDS CLOSED TO 100%% (180 Deg)------------------\n\r\n");

		get_light_intensity_values(ptr_data);
		get_damping_period(ptr_data);
		get_hystereses_band_values(ptr_data);
		get_user_scene_target_angle(ptr_data);

		printf("ENTER USER SCENE TARGET TIME TO OPEN BLINDS (24HR)(HH:MM:SS):               \r\n");
		get_user_scene_times(ptr_data, alarm_a);

		printf("ENTER USER SCENE TARGET TIME TO RETURN TO TARGET ANGLE (24HR)(HH:MM:SS):    \r\n");
		get_user_scene_times(ptr_data, alarm_b);

		printf("\n---------PRESS USER BUTTON TO CONTINUE TO AUTONOMOUS STATE--------\n\r\n");
		ptr_data->config_flag = STATE_CONFIG_COMPLETE;
	}

	if(ptr_data->bool_rising_edge)
	{
		ret_val = STATE_AUTONOMOUS_OPERATION;
		ptr_data->config_flag = STATE_CONFIG_INCOMPLETE;
	}

	return ret_val;
}

/**
 * @brief Function for collecting integer values and validating them
 *
 * Input is validated by:
 * Checking that fgets actually received something
 * Convert to long
 * Moving the end of the string past any whitespace so whitespace post input is accepted
 * Ensuring result is a numeric value
 * Ensuring result only has numeric values or whitespace
 * Ensuring value is in scope for a long variable
 * Ensuring value is in scope for a int variable then cast to int
 * Ensuring value is between min and max thresholds
 *
 * @param result_ptr Pointer passed through so result can be collected
 * @param min_value Minimum threshold so values collected are above the value passed
 * @param max_value Maximum threshold so values collected are below the value passed
 * @retval Input validation error check. If 0 input was successfully validated
 *
 */
static int get_int(int *result_ptr ,int min_value, int max_value)
{
	char input[STRING_BUFFER] = {0};
	char *end_ptr = 0;
	long int result = 0;
	int ret_val = 0;

	if (!fgets(input, sizeof(input), stdin)) {
		fprintf(stderr, "Error: input failure\r\n");
		ret_val = 1;
	}

	input[strcspn(input, "\n")] = '\0';

	errno = 0;

	result = strtol(input, &end_ptr, 10);

	while (*end_ptr == ' ' || *end_ptr == '\t')
		    end_ptr++;

	if (input == end_ptr) {
		fprintf(stderr, "Error: %s, is not a numeric value\r\n", input);
		ret_val = 1;
	}
	else if (*end_ptr != '\0') {
		fprintf(stderr, "Error: %s has extra non numeric characters after conversion, string is not a number\r\n", input);
		ret_val = 1;
	}
	else if (errno == ERANGE) {
		fprintf(stderr, "Error: %s when converted is out of range for long variable type\r\n", input);
		ret_val = 1;
	}
	else if (result > INT_MAX || result < INT_MIN) {
		fprintf(stderr, "Error: %s is outside of the integer range\r\n", input);
		ret_val = 1;
	}
	else {
		*result_ptr = (int)result;
	}

	if (!ret_val){
		if (*result_ptr > max_value) {
			fprintf(stderr, "Error: input was too high. Max Value:  %i\r\n", max_value);
			ret_val = 1;
		}

		if (*result_ptr < min_value) {
			fprintf(stderr, "Error: input was too low. Min Value:  %i\r\n", min_value);
			ret_val = 1;
		}
	}

    return ret_val;
}

/**
 * @brief Function for collecting float values and validating them
 *
 * Input is validated by:
 * Checking that fgets actually received something
 * Convert to double
 * Moving the end of the string past any whitespace so whitespace post input is accepted
 * Ensuring result is a numeric value
 * Ensuring result only has numeric values or whitespace
 * Ensuring value is in scope for a double variable then cast to float
 * Ensuring value is between min and max thresholds
 *
 * @param result_ptr Pointer passed through so result can be collected
 * @param min_value Minimum threshold so values collected are above the value passed
 * @param max_value Maximum threshold so values collected are below the value passed
 * @retval Input validation error check. If 0 input was successfully validated
 *
 */
static int get_float(float *result_ptr ,float min_value, float max_value)
{
	char input[STRING_BUFFER] = {0};
	char *end_ptr = 0;
	double result = 0;
	int ret_val = 0;

	if (!fgets(input, sizeof(input), stdin)) {
		fprintf(stderr, "Error: input failure\r\n");
		ret_val = 1;
	}

	input[strcspn(input, "\n")] = '\0';

	errno = 0;

	result = strtod(input, &end_ptr);

	while (*end_ptr == ' ' || *end_ptr == '\t')
	    end_ptr++;

	if (input == end_ptr) {
		fprintf(stderr, "Error: %s, is not a numeric value\r\n", input);
		ret_val = 1;
	}
	else if (*end_ptr != '\0') {
		fprintf(stderr, "Error: %s has extra non numeric characters after conversion, string is not a number\r\n", input);
		ret_val = 1;
	}
	else if (errno == ERANGE) {
		fprintf(stderr, "Error: %s when converted is out of range for double variable type\r\n", input);
		ret_val = 1;
	}
	else {
		*result_ptr = (float)result;
	}

	if (!ret_val){
		if (*result_ptr > max_value) {
			fprintf(stderr, "Error: input was too high. Max Value:  %.2f\r\n", max_value);
			ret_val = 1;
		}

		if (*result_ptr < min_value) {
			fprintf(stderr, "Error: input was too low. Min Value:  %.2f\r\n", min_value);
			ret_val = 1;
		}
	}

    return ret_val;
}

/**
 * @brief Function for collecting min and max light intensity values
 *
 * Function will ask for repeated input, until validated input is provided,
 * while get_float determines values are incorrect
 * Converts Volts to mV for integer storage
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval None
 *
 * @see get_float()
 */
static void get_light_intensity_values(MachineData *ptr_data)
{
	int valid_flag = 0;
	float result = 0;
	int mv = 0;

	printf("ENTER LIGHT INTENSITY VOLTS(V) (0-3.3V) MAX FOR FULLY CLOSED              \r\n");
	printf("RECOMMENDED VALUE 2.0V (FULL RANGE):\r\n");
		while(!valid_flag) {
			if (!get_float(&result, LIGHT_INTENSITY_MIN, LIGHT_INTENSITY_MAX))
			{
				ptr_data->light_intensity_max_mv = (int)(result * 1000);
				valid_flag = 1;
			} else {
				printf("ENTER VALUE AGAIN:                                                \r\n");
			}
		}

	valid_flag = 0;

	printf("ENTER LIGHT INTENSITY VOLTS(V) (0-3.3V) MIN FOR FULLY OPEN:                \r\n");
	printf("RECOMMENDED VALUE 1.68V (FULL RANGE):\r\n");
	while(!valid_flag) {
		if (!get_float(&result, LIGHT_INTENSITY_MIN, LIGHT_INTENSITY_MAX))
		{
			mv = (int)(result * 1000);

			if (mv < ptr_data->light_intensity_max_mv){
				ptr_data->light_intensity_min_mv = mv;
				valid_flag = 1;
			} else {
				printf("VALUE IS HIGHER THAN MAX. PLEASE ENTER AGAIN:                     \r\n");
			}

		} else {
			printf("ENTER VALUE AGAIN:                                                \r\n");
		}
	}
}

/**
 * @brief Function for collecting damping period
 *
 * Function will ask for repeated input, until validated input is provided,
 * while get_float determines values are incorrect
 * Converts seconds to milliseconds for integer storage
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval None
 *
 * @see get_float()
 */
static void get_damping_period(MachineData *ptr_data)
{
	int valid_flag = 0;
	float result = 0;

	printf("ENTER DAMPING TIME (0-6s) FOR SMOOTH MOVEMENT:        \r\n");
	while(!valid_flag) {
		if (!get_float(&result, DAMPING_PERIOD_MIN, DAMPING_PERIOD_MAX))
		{
			ptr_data->damping_period_ms = (int)(result * 1000);
			valid_flag = 1;
		} else {
			printf("ENTER VALUE AGAIN:                                                \r\n");
		}
	}
}

/**
 * @brief Function for collecting hysteresis max/min values
 *
 * Function will ask for repeated input, until validated input is provided,
 * while get_float determines values are incorrect
 * Converts Volts to mV for integer storage
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval None
 *
 * @see get_float()
 */
static void get_hystereses_band_values(MachineData *ptr_data)
{
	int valid_flag = 0;
	float result = 0;
	int mv = 0;

	printf("ENTER UPPER HYSTERESIS BAND VOLTS (0.1-1.0V):                             \r\n");
		while(!valid_flag) {
			if (!get_float(&result, HYSTERESES_BAND_MIN, HYSTERESES_BAND_MAX))
			{
				ptr_data->upper_hystereses_band_mv = (int)(result * 1000);
				valid_flag = 1;
			} else {
				printf("ENTER VALUE AGAIN:                                                \r\n");
			}
		}

	valid_flag = 0;

	printf("ENTER LOWER HYSTERESIS BAND VOLTS (0.1-1.0V):                             \r\n");
	while(!valid_flag) {
		if (!get_float(&result, HYSTERESES_BAND_MIN, HYSTERESES_BAND_MAX))
		{
			mv = (int)(result * 1000);
			ptr_data->lower_hystereses_band_mv = mv;
			valid_flag = 1;

		} else {
			printf("ENTER VALUE AGAIN:                                                \r\n");
		}
	}
}

/**
 * @brief Function for collecting scene state user set target value
 *
 * Function will ask for repeated input, until validated input is provided,
 * while get_int determines values are incorrect
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 * @retval None
 *
 * @see get_int()
 */
static void get_user_scene_target_angle(MachineData *ptr_data)
{
	int valid_flag = 0;
	int result = 0;

	printf("ENTER USER SCENE TARGET ANGLE DEGREE INTEGER (0-180):           \r\n");
	while(!valid_flag) {
		if (!get_int(&result, USER_TARGET_ANGLE_MIN, USER_TARGET_ANGLE_MAX))
		{
			ptr_data->scene_target_angle = result;
			valid_flag = 1;
		} else {
			printf("ENTER VALUE AGAIN:                                                \r\n");
		}
	}
}

/**
 * @brief Function for collecting scene state user set alarm times
 *
 * Function will ask for repeated input, until validated input is provided,
 * while get_int determines values are incorrect
 * Integer values for seconds, minutes and hours will be collected
 * Function then sets an alarm based on the parameter alarm_id
 * Function currently masks out hours and minutes for testing purposes
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 * @param alarm_id Int value to state which alarm to set
 * @retval None
 *
 * @see get_int()
 */
static void get_user_scene_times(MachineData *ptr_data, unsigned int alarm_id)
{
	int instance_valid_flag = 0;
	int result = 0;

	RTC_AlarmTypeDef alarm =  {0};
	int user_seconds = 0;
	int user_minutes = 0;
	int user_hours = 0;

	printf("ENTER HOURS:                                                      \r\n");
	while(!instance_valid_flag) {
		if (!get_int(&result, 0, MAX_HRS))
		{
			user_hours = result;
			instance_valid_flag = 1;
		} else {
			printf("ENTER VALUE AGAIN:                                                \r\n");
		}
	}

	instance_valid_flag = 0;

	printf("ENTER MINUTES:                                                    \r\n");
	while(!instance_valid_flag) {
		if (!get_int(&result, 0, MAX_MINUTES))
		{
			user_minutes = result;
			instance_valid_flag = 1;
		} else {
			printf("ENTER VALUE AGAIN:                                                \r\n");
		}
	}

	instance_valid_flag = 0;

	printf("ENTER SECONDS:                                                    \r\n");
		while(!instance_valid_flag) {
			if (!get_int(&result, 0, MAX_SECONDS))
			{
				user_seconds = result;
				instance_valid_flag = 1;
			} else {
				printf("ENTER VALUE AGAIN:                                                \r\n");
			}
		}

		alarm.AlarmTime.Hours = decimal_to_bcd(user_hours);
		alarm.AlarmTime.Minutes = decimal_to_bcd(user_minutes);
		alarm.AlarmTime.Seconds = decimal_to_bcd(user_seconds);
		alarm.AlarmMask = (RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES);
		alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
		alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
		alarm.AlarmDateWeekDay = 0x01;
		alarm.Alarm = alarm_id;

		/* Set Alarm to fire as interrupt (requires enabled NVIC IRQ) */
		HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BCD);
}
