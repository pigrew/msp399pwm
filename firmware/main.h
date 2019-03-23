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

#define WMIN 0x6a66
#define WMAX 0x6a66

#ifdef START_XTAL
#define PERIOD 0x9800
#else
#define PERIOD 0x6000
#endif


extern bool reportTemps;

#endif /* MAIN_H_ */
