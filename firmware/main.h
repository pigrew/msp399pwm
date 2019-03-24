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

#define XTAL_RATE (8000000ul)

#define FLLREF_RATE (4000000ul)
#define DCO_RATE (24000000ul)
#define ACLK_RATE (1500000ul)

#define TICK_RATE (1000u)

void main_set_tempReport(bool enabled);

#endif /* MAIN_H_ */
