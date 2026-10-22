/**
  ******************************************************************************
  * @file           : init.h
  * @author			: Steven Wheeldon
  * @version		: 1.0
  * @date			: 27/04/2026
  * @brief          : Header file for init.c
  *
  * This file holds all constants, function prototypes and global variables that
  * are needed for init.c and other files.
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

#ifndef INC_INIT_H_
#define INC_INIT_H_

/* Definitions ---------------------------------------------------------------*/
#define BUF_SIZ 1

/* Includes (Vendor Libraries, Standard Libraries, Personal Libraries) -------*/
#include "stm32f4xx_hal.h"
#include "state_machine.h"

/* Type Definitions ----------------------------------------------------------*/
extern void init_serial_io(void);

extern int init_gpio_btn(void);
extern int init_gpio_LED(void);
extern int init_pwm(void);
extern int init_gpio_adc(void);
extern int init_rtc(void);
extern void init_i2c(void);
extern void init_tim6(void);


/* Extern Function Prototypes ------------------------------------------------*/


/* Static Function Prototypes (Generated, Initialisation, Logic) -------------*/


/* Global Variables (Extern, Static) -----------------------------------------*/
extern TIM_HandleTypeDef  g_tim9;
extern ADC_HandleTypeDef g_adc_in;
extern RTC_HandleTypeDef hrtc;
extern I2C_HandleTypeDef g_isc1;
extern TIM_HandleTypeDef g_tim6;

extern unsigned int g_count;
extern unsigned int g_accu;

extern int scene_alarm_flag;

extern volatile unsigned char g_flag_pend_bus;
extern volatile unsigned char g_flag_recv_bus;
extern volatile unsigned char g_flag_errr_bus;
extern int tim6_period_elapsed;

extern int error_state_flag;
extern ErrorCause error_cause;


#endif /* INC_INIT_H_ */
