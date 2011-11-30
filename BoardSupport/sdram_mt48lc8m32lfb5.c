/**********************************************************************
* $Id$		sdram_mt48lc8m32lfb5.c			2011-06-02
*//**
* @file		sdram_mt48lc8m32lfb5.c
* @brief	Contains all functions support for Micron MT48LC8M32LFB5
* @version	1.0
* @date		02. June. 2011
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
#if(_CURR_USING_BRD == _QVGA_BOARD)
#ifdef _EMC
#include "bsp.h"
#include "lpc177x_8x_emc.h"
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_timer.h"
#include "sdram_mt48lc8m32lfb5.h"

/* Public Functions ----------------------------------------------------------- */
/** @addtogroup Sdram_MT48LC8M32FLB5
 * @{
 */

/*********************************************************************//**
 * @brief 		Initialize external SDRAM memory Micron MT48LC8M32LFB5
 *				256Mbit(8M x 32)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void SDRAMInit( void )
{
	uint32_t i, dwtemp;
	TIM_TIMERCFG_Type TIM_ConfigStruct;

	/* Initialize EMC */
	EMC_Init();
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 1;

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
	//Configure memory layout, but MUST DISABLE BUFFERs during configuration
	LPC_EMC->DynamicConfig0    = 0x00004480; /* 256MB, 8Mx32, 4 banks, row=12, column=9 */

	/*Configure timing for  Micron SDRAM MT48LC8M32LFB5-8 */

	//Timing for 48MHz Bus
	LPC_EMC->DynamicRasCas0    = 0x00000201; /* 1 RAS, 2 CAS latency */
	LPC_EMC->DynamicReadConfig = 0x00000001; /* Command delayed strategy, using EMCCLKDELAY */
	LPC_EMC->DynamicRP         = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */
	LPC_EMC->DynamicRAS        = 0x00000002; /* ( n + 1 ) -> 3 clock cycles */
	LPC_EMC->DynamicSREX       = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
	LPC_EMC->DynamicAPR        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
	LPC_EMC->DynamicDAL        = 0x00000002; /* ( n ) -> 2 clock cycles */
	LPC_EMC->DynamicWR         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
	LPC_EMC->DynamicRC         = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
	LPC_EMC->DynamicRFC        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
	LPC_EMC->DynamicXSR        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
	LPC_EMC->DynamicRRD        = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */
	LPC_EMC->DynamicMRD        = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */

	TIM_Waitms(100);						   /* wait 100ms */
	LPC_EMC->DynamicControl    = 0x00000183; /* Issue NOP command */

	TIM_Waitms(200);						   /* wait 200ms */
	LPC_EMC->DynamicControl    = 0x00000103; /* Issue PALL command */
	LPC_EMC->DynamicRefresh    = 0x00000002; /* ( n * 16 ) -> 32 clock cycles */

	for(i = 0; i < 0x80; i++);	           /* wait 128 AHB clock cycles */

	//Timing for 48MHz Bus
	LPC_EMC->DynamicRefresh    = 0x0000002E; /* ( n * 16 ) -> 736 clock cycles -> 15.330uS at 48MHz <= 15.625uS ( 64ms / 4096 row ) */

	LPC_EMC->DynamicControl    = 0x00000083; /* Issue MODE command */

	//Timing for 48/60/72MHZ Bus
	dwtemp = *((volatile uint32_t *)(SDRAM_BASE_ADDR | (0x22<<(2+2+9)))); /* 4 burst, 2 CAS latency */
	LPC_EMC->DynamicControl    = 0x00000000; /* Issue NORMAL command */

	//[re]enable buffers
	LPC_EMC->DynamicConfig0    = 0x00084480; /* 256MB, 8Mx32, 4 banks, row=12, column=9 */
}
#endif /*_EMC*/
#endif /*(_CURR_USING_BRD == _QVGA_BOARD)*/

/**
 * @}
 */

/*********************************************************************************
**                            End Of File
*********************************************************************************/
