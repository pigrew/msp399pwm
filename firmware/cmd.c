/*
 * cmd.c
 *
 *  Created on: Mar 22, 2019
 *      Author: nconrad
 */


#include "main.h"
#include "uart.h"
#include "cmd.h"
#include "pwm.h"

void processCmds() {
    uint32_t d32;
    uint8_t rsp = 0; // 0 is success

    if(!cmdComplete)
        return;
    if(rxBufLen <=1)
        goto end;
    // Add null terminator
    rxbuf[rxBufLen] = '\0';

    switch(rxbuf[0]) {
        case 't':
        case 'T':
            if(rxBufLen <=1)
                goto end;
            switch(rxbuf[1]) {
            case '0':
                reportTemps = false;
                break;
            case '1':
                reportTemps = true;
                break;
            }
            break;
        case 'p': // Set PWM period
        case 'P':
            d32 = strtoul((char*)&(rxbuf[1]), NULL, 0);
            if(d32 > 0)
                pwm_setPeriod((uint16_t)d32);
            break;
        case 'r': // Set PWM ratio
        case 'R':
            d32 = strtoul((char*)&(rxbuf[1]), NULL, 0);
            if(d32 > 0)
                pwm_setRatio(d32);
            break;
        default:
            rsp = 1;
            break;
    }

end:
    switch(rsp) {
    case 0:
        uart_write("OK\n", 3);
        break;
    case 1:
    default:
        uart_write("ERROR\n", 6);
        break;
    }
    rxBufLen = 0;
    cmdComplete = false;
    return;
}
