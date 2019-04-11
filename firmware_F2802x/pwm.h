/*
 * pwm.h
 *
 *  Created on: Apr 8, 2019
 *      Author: nconrad
 */

#ifndef PWM_H_
#define PWM_H_

#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "common/include/clk.h"
#include "common/include/gpio.h"

extern int MEP_ScaleFactor;

void pwm_init(CLK_Handle myClk, GPIO_Handle myGpio);
void pwm_tick();
void pwm_setRatio(uint32_t ratio);
void pwm_applyWhole(uint16_t x);
void pwm_applyMEP(uint16_t x);
#endif /* PWM_H_ */
