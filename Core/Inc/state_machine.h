/**
  ******************************************************************************
  * @file           : state_machine.h
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Header file for all 5 state machine .c files
  *
  * This file contains all the constans, function prototypes, struct information
  * and state function prototypes themselves.
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
  *
  ******************************************************************************
  */

#ifndef SRC_STATES_SCENE_OPERATION_STATE_H_
#define SRC_STATES_SCENE_OPERATION_STATE_H_

/* Definitions ---------------------------------------------------------------*/
#define GREENLED 12
#define ORANGELED 13
#define REDLED 14
#define BLUELED 15
#define RESET_LED 0

#define AUTO_STATE_LED_ROTATION_MS 250
#define MANUAL_STATE_LED_ROTATION_MS 500
#define SCENE_STATE_LED_ROTATION_MS 500
#define ERROR_STATE_LED_ROTATION_MS 125

#define LIVE_DATA_PRINT_MS 2500

#define STRING_BUFFER 10

#define LIGHT_INTENSITY_MIN 0.0f
#define LIGHT_INTENSITY_MAX 3.3f

#define DAMPING_PERIOD_MIN 0
#define DAMPING_PERIOD_MAX 6

#define HYSTERESES_BAND_MIN 0.1f
#define HYSTERESES_BAND_MAX 1.0f

#define USER_TARGET_ANGLE_MIN 0
#define USER_TARGET_ANGLE_MAX 180

#define MAX_SECONDS 59
#define MAX_MINUTES 59
#define MAX_HRS 24

#define BLINDS_OPEN_MS 0.5f
#define BLINDS_CLOSED_MS 2.5f

#define CRR_TICKS_PER_MS 80.0f

#define I2C_MAX_ERROR_COUNT 50
#define I2C_NO_DATA 255

#define STATE_CONFIG_COMPLETE 1
#define STATE_CONFIG_INCOMPLETE 0



/* Includes (Vendor Libraries, Standard Libraries, Personal Libraries) -------*/
#include "stm32f4xx_hal.h"

/* Type Definitions ----------------------------------------------------------*/
typedef enum
{
	STATE_INITIAL,
	STATE_AUTONOMOUS_OPERATION,
	STATE_MANUAL_OPERATION,
	STATE_SCENE_OPERATION,
	STATE_ERROR
} State;

typedef enum
{
	NO_ERROR,
	INIT_FAIL,
	I2C_FAIL
}ErrorCause;

typedef struct StructMachineData
{
	unsigned int last_tick;
	unsigned char last_bit;
	unsigned int bool_rising_edge;

	/*PwM Variables*/
	float pwm_output_ms;
	float pwm_stable_output_ms;
	float pwm_target_output_ms;

	unsigned int last_tick_print;
	int config_flag;

	/*Autonomous State Variables*/
	int light_intensity_max_mv;
	int light_intensity_min_mv;
	int damping_period_ms;
	int upper_hystereses_band_mv;
	int lower_hystereses_band_mv;
	int scene_target_angle;

	/*ADC Variables*/
	int current_adc_mv;
	int current_adc_percent;
	int max_adc_percent;
	int sum_adc_percent;
	int count_adc;
	int avg_adc_percent;

	/*I2C Variables*/
	unsigned char buff;
	unsigned int err;

	int i2c_error_count;

} MachineData;

typedef State func_state_t(MachineData*);

/* Extern Function Prototypes ------------------------------------------------*/
extern State machine(State, MachineData*);

extern State state_initial(MachineData*);

extern State state_autonomous_operation(MachineData*);

extern State state_manual_operation(MachineData*);

extern State state_scene_operation(MachineData*);

extern State state_error(MachineData*);

extern void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


/* Static Function Prototypes (Generated, Initialisation, Logic) -------------*/


/* Global Variables (Extern, Static) -----------------------------------------*/

#endif /* SRC_STATES_SCENE_OPERATION_STATE_H_ */
