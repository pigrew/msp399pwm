/*
 * cmd.c
 *
 *  Created on: Apr 10, 2019
 *      Author: nconrad
 */
#include <stdint.h>

//#include "main.h"
#include "uart.h"
#include "pwm.h"

static void write_temps(void) {

    char str[8];
    //uint16_t lt = tmp411_getLocal();
    //uint16_t rt = tmp411_getRemote();
    uint16_t lt=12, rt=12;
    uart_write("T",1);
    u16hex(lt,str,16);
    uart_write(str, 4);
    uart_write(",", 1);
    u16hex(rt,(char*)str,16);
    uart_write(str, 4);
    uart_write("\n",1);
}
static uint32_t atoul(char *str);
void processCmds() {
    uint32_t d32;
    uint16_t rsp = 0; // 0 is success

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
                //main_set_tempReport(false);
                break;
            case '1':
                //main_set_tempReport(true);
                break;
            case '?':
                rsp=2;
                break;
            }
            break;
        case 'p': // Set PWM period
        case 'P':
            //d32 = strtoul((char*)&(rxbuf[1]), NULL, 0);
            d32 = atoul((char*)&(rxbuf[1]));
            if(d32 > 0x1800) // bad things happen with sigma-delta if it goes too fast
                ;//pwm_setPeriod((uint16_t)d32);
            else
                rsp = 1;
            break;
        case 'r': // Set PWM ratio
        case 'R':
            d32 = atoul((char*)&(rxbuf[1]));
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
// atoi is for signed.... this one is for unsigned.
// No error handling (for now)
static uint32_t atoul(char *str) {
    uint32_t res = 0;

    // Iterate through all characters of input string and
    // update result
    while (*str != '\0') {
        res = res*10 + (*str - '0');
        str++;
    }

    // return result.
    return res;
}
