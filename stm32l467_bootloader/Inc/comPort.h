/*
 * comPort.h
 *
 *  Created on: 5 Mar 2019
 *      Author: emirs
 */

#ifndef COMPORT_H_
#define COMPORT_H_


#include "stdint.h"
#include "stm32l4xx_hal.h"
#include "stdarg.h"
#include "string.h"
#include "main.h"
#include "stdio.h"

#define D_UART				&huart3							//
#define C_UART				&huart2							//Command UART


void printMsg(uint8_t *frmt, ...);

#endif /* COMPORT_H_ */
