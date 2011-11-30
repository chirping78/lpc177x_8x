/**********************************************************************
* $Id$		usbhost_uart.h			2011-09-05
*//**
* @file		usbhost_uart.h
* @brief		Provide APIs for Printing debug information.
* @version	1.0
* @date		05. September. 2011
* @author	NXP MCU SW Application Team
* 
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
**********************************************************************/


#ifndef USBHOST_UART_H
#define USBHOST_UART_H
#include "lpc_types.h"
/*
**************************************************************************************************************
*                                           INCLUDE HEADER FILES
**************************************************************************************************************
*/

#include <stdarg.h>
#include <stdio.h>
#include "usbhost_inc.h"

/** @addtogroup USBHost_Uart
 * @{
 */

/*********************************************************************//**
 *   PRINT CONFIGURATION
 **********************************************************************/
#define  PRINT_ENABLE         1

#if PRINT_ENABLE
#define  PRINT_Log(...)       UART_Printf(__VA_ARGS__)
#define  PRINT_Err(rc)        UART_Printf("ERROR: In %s at Line %u - rc = %d\n", __FUNCTION__, __LINE__, rc)

#else 
#define  PRINT_Log(...)       do {} while(0)
#define  PRINT_Err(rc)        do {} while(0)

#endif


/* Public Functions ----------------------------------------------------------- */
/** @defgroup FAT Public Functions
 * @{
 */

void  UART_PrintChar (      uint8_t   ch);
void  UART_PrintStr  (const uint8_t  *str);
void  UART_Printf(const void  *format, ...);

/**
 * @}
 */
 /**
 * @}
 */

#endif
