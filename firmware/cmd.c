/*
 * cmd.c
 *
 *  Created on: Mar 22, 2019
 *      Author: nconrad
 */


#include "main.h"
#include "uart.h"

void processCmds() {
    if(!cmdComplete)
        return;
    if(rxBufLen <=1)
        goto end;
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
    }

end:
    rxBufLen = 0;
    cmdComplete = false;
    return;
}
