/*
 * MyPrintf.h
 *
 *
 *  Created on: Sep 23, 2022
 *      Author: Kino Van Tin
 */

#ifndef MYPRINTF_H_
#define MYPRINTF_H_

#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <stdarg.h>

#define DebugUART huart1
#define PrintBufferSize 70

extern UART_HandleTypeDef DebugUART;

void 	SyncPrintf_Init();
int		SyncPrintf (const char *__restrict format, ...)
               _ATTRIBUTE ((__format__ (__printf__, 1, 2)));


#endif /* MYPRINTF_H_ */
