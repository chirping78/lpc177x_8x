/**********************************************************************
* $Id$		sdram_is42s32800d.c			2011-08-22
*//**
* @file		sdram_is42s32800d.c
* @brief	Contains all functions support for ISSI IS42S32800D
* @version	1.0
* @date		22. August. 2011
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
#include "bsp.h"
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc177x_8x_libcfg.h"
#else
#include "lpc177x_8x_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#if (_CURR_USING_BRD == _EA_PA_BOARD)
#ifdef _EMC

#include "bsp.h"
#include "lpc177x_8x_emc.h"
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_timer.h"
#include "sdram_is42s32800d.h"

/* Public Functions ----------------------------------------------------------- */
/** @addtogroup Sdram_IS42S32800D
 * @{
 */


/*********************************************************************
 * @brief		Calculate refresh timer (the multiple of 16 CCLKs)
 * @param[in]	freq - frequency of EMC Clk
 * @param[in]	time - micro second
 * @return 		None
 **********************************************************************/
#define EMC_SDRAM_REFRESH(freq,time)  \
  (((uint64_t)((uint64_t)time * freq)/16000000ull)+1)

/*********************************************************************
 * @brief		Calculate EMC Clock from nano second
 * @param[in]	freq - frequency of EMC Clk
 * @param[in]	time - nano second
 * @return 		None
 **********************************************************************/
uint32_t NS2CLK(uint32_t freq,uint32_t time){
 return (((uint64_t)time*freq/1000000000ull));
}

/*********************************************************************//**
 * @brief 		Initialize external SDRAM memory ISSI IS42S32800D
 *				256Mbit(8M x 32)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void SDRAMInit( void )
{
	uint32_t i;
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	uint32_t emc_freq;

	/* Initialize EMC */
	EMC_Init();
	emc_freq = CLKPWR_GetCLK(CLKPWR_CLKTYPE_EMC);

	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 1;

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
	//Configure memory layout, but MUST DISABLE BUFFERs during configuration
	LPC_EMC->DynamicConfig0    = 0x00004480; /* 256MB, 8Mx32, 4 banks, row=12, column=9 */
	/*Configure timing for  ISSI IS42S32800D-7 */

	//Timing for 80MHz Bus
	LPC_EMC->DynamicRasCas0    = 0x00000201; /* 1 RAS, 2 CAS latency */
	LPC_EMC->DynamicReadConfig = 0x00000001; /* Command delayed strategy, using EMCCLKDELAY */
	LPC_EMC->DynamicRP         = NS2CLK(emc_freq, 20);
	LPC_EMC->DynamicRAS        = NS2CLK(emc_freq, 45);
	LPC_EMC->DynamicSREX       = NS2CLK(emc_freq, 70);
	LPC_EMC->DynamicAPR        = 0x00000005;
	LPC_EMC->DynamicDAL        = 0x00000005;
	LPC_EMC->DynamicWR         = 2;
	LPC_EMC->DynamicRC         = NS2CLK(emc_freq, 68);
	LPC_EMC->DynamicRFC        = NS2CLK(emc_freq, 68);
	LPC_EMC->DynamicXSR        = NS2CLK(emc_freq, 70);
	LPC_EMC->DynamicRRD        = NS2CLK(emc_freq, 14);
	LPC_EMC->DynamicMRD        = 0x00000002;

	TIM_Waitms(100);						   /* wait 100ms */
	LPC_EMC->DynamicControl    = 0x00000183; /* Issue NOP command */

	TIM_Waitms(200);						   /* wait 200ms */
	LPC_EMC->DynamicControl    = 0x00000103; /* Issue PALL command */
	LPC_EMC->DynamicRefresh    = 0x00000002; /* ( n * 16 ) -> 32 clock cycles */

	for(i = 0; i < 0x80; i++);	           /* wait 128 AHB clock cycles */

	LPC_EMC->DynamicRefresh    = EMC_SDRAM_REFRESH(emc_freq, 64);

	LPC_EMC->DynamicControl    = 0x00000083; /* Issue MODE command */

	//Timing for 48/60/72MHZ Bus
	LPC_EMC->DynamicControl    = 0x00000000; /* Issue NORMAL command */

	//[re]enable buffers
	LPC_EMC->DynamicConfig0    = 0x00084480; /* 256MB, 8Mx32, 4 banks, row=12, column=9 */
}

#endif /*_EMC*/
#endif /*(_CURR_USING_BRD == _EA_PA_BOARD)*/
/**
 * @}
 */

/*********************************************************************************
**                            End Of File
*********************************************************************************/
