/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : main.c
 *    Description : Main module
 *
 *    History :
 *    1. Date        : 4, August 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *  This example project shows how to use the IAR Embedded Workbench for ARM
 * to develop code for the IAR LPC-1788 board. It shows basic use of the I/O,
 * the timer and the interrupt controllers.
 *  It starts by blinking USB Link LED.
 *
 * Jumpers:
 *  EXT/JLINK  - depending of power source
 *  ISP_E      - unfilled
 *  RST_E      - unfilled
 *  BDS_E      - unfilled
 *  C/SC       - SC
 *
 * Note:
 *  After power-up the controller get clock from internal RC oscillator that
 * is unstable and may fail with J-Link auto detect, therefore adaptive clocking
 * should always be used. The adaptive clock can be select from menu:
 *  Project->Options..., section Debugger->J-Link/J-Trace  JTAG Speed - Adaptive.
 *
 *    $Revision: 24636 $
 **************************************************************************/
#include <stdio.h>
#include "board.h"
#include "sdram_k4s561632j.h"
#include "drv_glcd.h"
#include "logo.h"
#include "Cursor.h"
#include "smb380_drv.h"
#include "system_LPC177x_8x.h"
#include "lpc177x_8x_clkpwr.h"
#include "bsp.h"


/** @defgroup LCD_GFT035A320240Y	LCD GFT035A320240Y
 * @ingroup LCD_Examples
 * @{
 */
 
#define TIMER0_TICK_PER_SEC   10
#define BEEP 	 (1 << 30)

extern uint32_t LCD_VRAM_BASE_ADDR;
//#define LCD_VRAM_BASE_ADDR ((uint32_t)&SDRAM_BASE_ADDR)

uint8_t Smb380Id, Smb380Ver;

#if (_CURR_USING_BRD != _IAR_OLIMEX_BOARD)
//dummy function, defined to avoid linking error when define wrong board
void SDRAMInit( void ){}  
#endif


/*************************************************************************
 * Function Name: Timer0IntrHandler
 * Parameters: none
 *
 * Return: none
 *
 * Description: Timer 0 interrupt handler
 *
 *************************************************************************/
//void Timer0IntrHandler (void)
void TIMER0_IRQHandler(void)
{
  // Toggle USB Link LED
  USB_D_LINK_LED_FIO ^= USB_D_LINK_LED_MASK;
  // clear interrupt
  LPC_TIM0->IR |=(1<<0);
}

void DelayNS(uint32_t dly)
{
	uint32_t i = 0;

	for ( ; dly > 0; dly--)
		for (i = 0; i < 5000; i++);
}

/*************************************************************************
 * Function Name: main
 * Parameters: none
 *
 * Return: none
 *
 * Description: main
 *
 *************************************************************************/
int main(void)
{
  typedef uint32_t ram_unit;
  uint32_t i;
  SMB380_Data_t XYZT;
  int cursor_x = (C_GLCD_H_SIZE - CURSOR_H_SIZE)/2, cursor_y = (C_GLCD_V_SIZE - CURSOR_V_SIZE)/2;

  // SDRAM Init	= check right board to avoid linking error
  SDRAMInit();

  GLCD_Ctrl (FALSE);
  GLCD_Init (LogoPic.pPicStream, NULL);

  GLCD_Cursor_Dis(0);

  GLCD_Copy_Cursor ((uint32_t *)Cursor, 0, sizeof(Cursor)/sizeof(uint32_t));

  GLCD_Cursor_Cfg(CRSR_FRAME_SYNC | CRSR_PIX_64);

  GLCD_Move_Cursor(cursor_x, cursor_y);

  GLCD_Cursor_En(0);

  // Init USB Link  LED
  USB_D_LINK_LED_FDIR = USB_D_LINK_LED_MASK;
  USB_D_LINK_LED_FSET = USB_D_LINK_LED_MASK;

  // Enable TIM0 clocks
  CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0, ENABLE);

  // Init Time0
  LPC_TIM0->TCR &= ~(1<<0);
  LPC_TIM0->TCR |= (1<<1);
  LPC_TIM0->TCR &= ~(1<<1);
  LPC_TIM0->CTCR &= ~(0x03<<0);
  LPC_TIM0->MCR |= (1<<0);
  LPC_TIM0->MCR |= (1<<1);
  LPC_TIM0->MCR &= ~(1<<2);

  // set timer 0 period
  LPC_TIM0->PR = 0;
  LPC_TIM0->MR0 = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER)/(TIMER0_TICK_PER_SEC);
  // init timer 0 interrupt
  LPC_TIM0->IR |=(1<<0);
  NVIC_EnableIRQ(TIMER0_IRQn);
  LPC_TIM0->TCR |= (1<<0);
  GLCD_Ctrl (TRUE);

  /* Initialize accel?ation sensor */
  SMB380_Init();

  SMB380_GetID(&Smb380Id, &Smb380Ver);

  while(1)
  {
    for(i = 0; i < 100000;  i++);

    SMB380_GetData (&XYZT);

    cursor_x += XYZT.AccX/512;
    cursor_y += XYZT.AccY/512;

    if((C_GLCD_H_SIZE - CURSOR_H_SIZE/2) < cursor_x)
    {
      cursor_x = C_GLCD_H_SIZE - CURSOR_H_SIZE/2;
    }

    if(-(CURSOR_H_SIZE/2) > cursor_x)
    {
      cursor_x = -(CURSOR_H_SIZE/2);
    }

    if((C_GLCD_V_SIZE - CURSOR_V_SIZE/2) < cursor_y)
    {
      cursor_y = (C_GLCD_V_SIZE - CURSOR_V_SIZE/2);
    }

    if(-(CURSOR_V_SIZE/2) > cursor_y)
    {
      cursor_y = -(CURSOR_V_SIZE/2);
    }

    GLCD_Move_Cursor(cursor_x, cursor_y);
  }

  while(1);
  return 1;
}


/**
 * @}
 */
