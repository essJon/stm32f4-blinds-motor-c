/**
  ******************************************************************************
  * @file           : util.c
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Utility functions for program
  *
  * This file holds a number of conversion functions for changing value types to
  * others, it also holds the function for debouncing button presses, as well as
  * smoothing pulse width modulation to ensure smooth servo control.
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

/* Includes (Vendor Libraries, Standard Libraries, Personal Libraries) -------*/
#include <math.h>

#include "util.h"

/**
 * @brief Function to poll for button presses and apply debouncing algorithm
 *
 * Function checks if button is pressed and different from previous state and grabs tick to start counting for the
 * debounce period. When the period has elapsed, the last bounce count is reset and the steady state is changed to indicate a button press
 * or depress
 *
 * @param last_bounce_ms Pointer to the timestamp when the last state change was detected and was different to the previous state
 * @param last_bouncable_state Pointer to the last state the button was in
 * @param last_steady_state Pointer to the debouncing stable state to return
 * @retval Post debouncing stable state to return
 *
 * @see program_loop()
 */
GPIO_PinState debounced_button_poll(unsigned int* last_bounce_ms, GPIO_PinState *last_bounceable_state, GPIO_PinState *last_steady_state)
{
	/* Local Variable */
	GPIO_PinState current_read = GPIOA->IDR & 1;
	unsigned int current_ms = HAL_GetTick();

	if(current_read != *last_bounceable_state)
	{
		/* State Changed - Grab Tick to Eliminate Bounce */
		*last_bounce_ms = current_ms;
		*last_bounceable_state = current_read;
	}
	else if(*last_bounce_ms && (current_ms - *last_bounce_ms) > DEBOUNCEMS)
	{
		/* Nominal Bounce Time Elapsed - State Considered Stable */
		*last_bounce_ms = 0;
		/* Update Last State */
		*last_steady_state = current_read;
	}

/* Return Current State */
return *last_steady_state;
}

/**
 * @brief Function to smooth motor movement according to hysteresis bands and damping period
 *
 * Function first ensures the new output ms is outside of the hysteresis bands, then changes target if so.
 * Next the target ms is broken down into fractional steps to ensure smooth movement based on the damping period and the 1 ms
 * update limit. Finally the new stable output is converted to CCR register by multiplying by the amount of CCR ticks
 * per ms (80).
 *
 * @param ptr_data Pointer to shared persistent state machine struct
 * @param pwm_tick Pointer to a timestamp so the function can only update CCR once every 1ms
 * @retval None
 *
 * @see program_loop()
 * @see init_pwm()
 */
void process_motor_smoothing(MachineData *ptr_data, unsigned int *pwm_tick)
{

	if (ptr_data->pwm_output_ms > ptr_data->pwm_target_output_ms + mv_to_ms(ptr_data->upper_hystereses_band_mv))
	{
		ptr_data->pwm_target_output_ms = ptr_data->pwm_output_ms;
	}
	else if(ptr_data->pwm_output_ms < ptr_data->pwm_target_output_ms - mv_to_ms(ptr_data->lower_hystereses_band_mv))
	{
		ptr_data->pwm_target_output_ms = ptr_data->pwm_output_ms;
	}

	if (ptr_data->pwm_target_output_ms != ptr_data->pwm_stable_output_ms && HAL_GetTick() - *pwm_tick >= ONE_MS)
	{
		ptr_data->pwm_stable_output_ms += (ptr_data->pwm_target_output_ms - ptr_data->pwm_stable_output_ms) / ptr_data->damping_period_ms;
		*pwm_tick = HAL_GetTick();
	}

	g_tim9.Instance->CCR1 = ptr_data->pwm_stable_output_ms * CRR_TICKS_PER_MS;
}

/**
 * @brief Function to calculate pulse ms from a given target angle in degrees
 * @param target_angle The angle to be converted to ms as an int
 * @retval The final ms value as a float
 */
float calculate_pulse_ms(int target_angle)
{
	return ((float)target_angle / (float)SCALING_FACTOR) + ZERO_DEGREE_PULSE_MS;
}

/**
 * @brief Function to calculate blind angle from a given ms value
 * @param ms Float value to be converted to int angle
 * @retval The final angle as an int
 */
int calculate_blind_angle(float ms)
{
	return (int)ceil((ms - ZERO_DEGREE_PULSE_MS) * SCALING_FACTOR);
}

/**
 * @brief Function to calculate ms from a given int mV value
 * @param mv The mV value to be converted to ms
 * @retval The final ms value as a float
 */
float mv_to_ms(int mv)
{
	return (float) mv/FULL_MOTOR_RANGE_MV  * PULSE_RANGE_MS;
}

/**
 * @brief Function to calculate Binary Coded Decimal(BCD) from a given decimal value
 * @param val The decimal integer to be converted to BCD
 * @retval The final BCD as an 8 bit integer, or char
 */
unsigned char decimal_to_bcd(int val)
{
    return ((val / 10) << 4) | (val % 10);
}
