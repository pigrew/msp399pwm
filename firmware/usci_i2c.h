/*
 * usci_i2c.h
 *
 *  Created on: Mar 29, 2019
 *      Author: nconrad
 */

#ifndef USCI_I2C_H_
#define USCI_I2C_H_

void ucb_i2c_init(
        uint16_t ucb_i2c_base, uint16_t port_base,
        uint8_t sda_pin, uint8_t scl_pin, // Pin # (not bit field)
        uint16_t clk_source, // UCSSEL__UCLK, UCSSEL__ACLK , UCSSEL__SMCLK
        uint16_t clock_div);

void ucb_i2c_writeReg8(uint16_t addr, uint8_t reg, uint8_t value);
uint16_t ucb_i2c_readReg16(uint16_t addr, uint8_t reg);

#endif /* USCI_I2C_H_ */
