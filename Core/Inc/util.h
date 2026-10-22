/**
  ******************************************************************************
  * @file           : util.h
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Header file for util.c
  *
  * This file holds all constants and function prototypes are needed for util.c
  * and other files.
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

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

/* Definitions ---------------------------------------------------------------*/
#define DEBOUNCEMS 20
#define SCALING_FACTOR 90
#define ZERO_DEGREE_PULSE_MS 0.5f
#define ONE_MS 1
#define FULL_MOTOR_RANGE_MV 5000.0f
#define PULSE_RANGE_MS 2.0f

/* Includes (Vendor Libraries, Standard Libraries, Personal Libraries) -------*/
#include "stm32f4xx_hal.h"
#include "state_machine.h"
#include "init.h"

/* Type Definitions ----------------------------------------------------------*/

/* Extern Function Prototypes ------------------------------------------------*/
extern GPIO_PinState debounced_button_poll(unsigned int* last_bounce_ms, GPIO_PinState *last_bounceable_state, GPIO_PinState *last_steady_state);
extern void process_motor_smoothing(MachineData *ptr_data, unsigned int *pwm_tick);
extern float calculate_pulse_ms(int target_angle);
extern int calculate_blind_angle(float ms);
extern float mv_to_ms(int mv);
extern unsigned char decimal_to_bcd(int val);

/* Static Function Prototypes (Generated, Initialisation, Logic) -------------*/


/* Global Variables (Extern, Static) -----------------------------------------*/




#endif /* SRC_UTIL_H_ */
