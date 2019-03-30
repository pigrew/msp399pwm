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
#include "tmp411.h"

static void write_temps(void) {

    uint8_t str[8];
    uint16_t lt = tmp411_getLocal();
    uint16_t rt = tmp411_getRemote();
    uart_write("T",1);
    u16hex(lt,(char*)str,16);
    uart_write(str, 4);
    uart_write(",", 1);
    u16hex(rt,(char*)str,16);
    uart_write(str, 4);
    uart_write("\n",1);
}
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
                main_set_tempReport(false);
                break;
            case '1':
                main_set_tempReport(true);
                break;
            case '?':
                rsp=2;
                break;
            }
            break;
        case 'p': // Set PWM period
        case 'P':
            d32 = strtoul((char*)&(rxbuf[1]), NULL, 0);
            if(d32 > 0x1800) // bad things happen with sigma-delta if it goes too fast
                pwm_setPeriod((uint16_t)d32);
            else
                rsp = 1;
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
    case 2:
        write_temps();
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
