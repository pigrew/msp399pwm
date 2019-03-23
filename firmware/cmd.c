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
    }

end:
    rxBufLen = 0;
    cmdComplete = false;
    return;
}
