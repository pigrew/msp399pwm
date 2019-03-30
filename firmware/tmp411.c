/*
 * tmp411.c
 *
 *  Created on: Mar 22, 2019
 *      Author: nconrad
 */

#define TMP411_ADDR 0x4c

#include <stdlib.h>
#include <stdint.h>
#include <msp430.h>
#include "usci_i2c.h"

#define I2C_BASE USCI_B0_BASE

#define TMP411_REG_LT_HIGH_RD 0x00
#define TMP411_REG_LT_LOW_RD 0x15

#define TMP411_REG_RT_HIGH_RD 0x01
#define TMP411_REG_RT_LOW_RD 0x10

#define TMP411_REG_RES 0x1a
#define TMP411_REG_RES_9 (0x1c | 0x00)
#define TMP411_REG_RES_10 (0x1c | 0x01)
#define TMP411_REG_RES_11 (0x1c | 0x10)
#define TMP411_REG_RES_12 (0x1c | 0x11)

#define TMP411_REG_RATE_RD 0x04
#define TMP411_REG_RATE_WR 0x0a
#define TMP411_REG_RATE_0p25 (0x02)
#define TMP411_REG_RATE_0p5 (0x03)

#define TMP411_REG_RESET_WR 0xFC

void tmp411_init() {

    ucb_i2c_init(USCI_B0_BASE,
                 PA_BASE, /* SDA = */ 4 , /* SCL = */ 5,
                 UCSSEL__ACLK,  60u);
   ucb_i2c_writeReg8(TMP411_ADDR, TMP411_REG_RESET_WR, 0x00);
   ucb_i2c_writeReg8(TMP411_ADDR, TMP411_REG_RES, TMP411_REG_RES_12);
   ucb_i2c_writeReg8(TMP411_ADDR, TMP411_REG_RATE_WR, TMP411_REG_RATE_0p5);
}

uint16_t tmp411_getLocal() {
    return ucb_i2c_readReg16(TMP411_ADDR,TMP411_REG_LT_HIGH_RD);
}
uint16_t tmp411_getRemote() {
    return ucb_i2c_readReg16(TMP411_ADDR,TMP411_REG_RT_HIGH_RD);
}
