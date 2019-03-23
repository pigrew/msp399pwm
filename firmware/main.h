/*
 * main.h
 *
 *  Created on: Mar 21, 2019
 *      Author: nconrad
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>

#define START_XTAL

#define ACLK_RATE (1500000ul)

#define TICK_RATE (1000u)

#define WMIN 0x6a66
#define WMAX 0x6a66

#ifdef START_XTAL
#define PERIOD 0x9540
#else
#define PERIOD 0x6000
#endif

void main_set_tempReport(bool enabled);

#endif /* MAIN_H_ */
