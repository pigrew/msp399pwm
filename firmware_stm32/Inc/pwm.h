/*
 * pwm.h
 *
 *  Created on: Apr 1, 2019
 *      Author: nconrad
 */

#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

void pwm_init();
void pwm_setPeriod(uint16_t period);
void pwm_setRatio(uint32_t ratio);

#endif /* PWM_H_ */
