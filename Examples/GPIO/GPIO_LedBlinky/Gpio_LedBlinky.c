/**********************************************************************
* $Id$      Gpio_LedBlinky.c        2011-06-02
*//**
* @file     Gpio_LedBlinky.c
* @brief    This example describes how to use GPIO interrupt to drive
*           LEDs
* @version  1.0
* @date     02. June. 2011
* @author   NXP MCU SW Application Team
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
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_clkpwr.h"
#include "bsp.h"


/** @defgroup GPIO_LedBlinky    GPIO Blinky
 * @ingroup GPIO_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/
/* SysTick Counter */
volatile unsigned long SysTickCnt;

/************************** PRIVATE FUNCTIONS *************************/
void SysTick_Handler (void);

void Delay (unsigned long tick);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       SysTick handler sub-routine (1ms)
 * @param[in]   None
 * @return      None
 **********************************************************************/
void SysTick_Handler (void)
{
    SysTickCnt++;
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Delay function
 * @param[in]   tick - number milisecond of delay time
 * @return      None
 **********************************************************************/
void Delay (unsigned long tick)
{
    unsigned long systickcnt;

    systickcnt = SysTickCnt;
    while ((SysTickCnt - systickcnt) < tick);
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry (void)
{
    uint8_t value = 0;
    uint32_t cclk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_CPU);

    /* Generate interrupt each 1 ms   */
    SysTick_Config(cclk/1000 - 1);

    GPIO_Init();

    GPIO_SetDir(BRD_LED_1_CONNECTED_PORT, BRD_LED_1_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);

    while (1)
    {
        GPIO_OutputValue(BRD_LED_1_CONNECTED_PORT, BRD_LED_1_CONNECTED_MASK, value);

        value = !value;
        Delay(500);
    }

}


/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    c_entry();
    return 0;
}

/*
 * @}
*/
