/*
 * pwm.h
 *
 *  Created on: Apr 8, 2019
 *      Author: nconrad
 */

#ifndef PWM_H_
#define PWM_H_

#include "common/include/clk.h"
#include "common/include/gpio.h"

void pwm_init(CLK_Handle myClk, GPIO_Handle myGpio);

#endif /* PWM_H_ */
