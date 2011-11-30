/**********************************************************************
* $Id$		Adc_Burst.c		2011-06-02
*//**
* @file		Adc_Burst.c
* @brief	This example describes how to use ADC conversion in
* 			burst mode
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
#include "lpc_types.h"
#include "lpc177x_8x_adc.h"
#include "lpc177x_8x_pinsel.h"
#include "debug_frmwrk.h"
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_exti.h"
#include "bsp.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup Adc_Burst		ADC Burst
 * @ingroup ADC_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS ***********************/
#define LPC177x_8x_ADC_INJECT_TEST
#define LPC177x_8x_ADC_BURST_MULTI

#ifdef LPC177x_8x_ADC_BURST_MULTI
#define _ADC_INT_n			ADC_ADINTEN3
#define _ADC_CHANNEL_n		ADC_CHANNEL_3
#endif


/** DMA size of transfer */
#define DMA_SIZE		8

#ifdef LPC177x_8x_ADC_INJECT_TEST
#define GPIO_INT	(1<<10)
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
#define LED_PORT	(1)			// P1.18 (LED USB Host) is used as polling LED when inject other ADC channel
#define LED_PIN		(1<<18)
#else
#define LED_PORT	(0)			// P0.13 (LED USB Host) is used as polling LED when inject other ADC channel
#define LED_PIN		(1<<13)
#endif
#endif /* (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)*/

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" ADC burst demo \n\r"
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
"\t - Communicate via: UART0 - 115200 bps \n\r"
" Use ADC with 12-bit resolution rate of 400KHz, running burst mode (single \n\r"
" or multiple input)\n\r"
" Display ADC value via UART0\n\r"
" Turn the potentiometer to see how ADC value changes\n\r"
" Press q to stop the demo\n\r"
"********************************************************************************\n\r";

#ifdef LPC177x_8x_ADC_INJECT_TEST
static BOOL_8 toggle=FALSE;
#endif
/************************** PRIVATE FUNCTION *************************/
void print_menu(void);

#ifdef LPC177x_8x_ADC_INJECT_TEST
void EINT0_IRQHandler(void);
#endif

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
#ifdef LPC177x_8x_ADC_INJECT_TEST
/*********************************************************************//**
 * @brief		Print menu
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void EINT0_IRQHandler(void)
{
	EXTI_ClearEXTIFlag(EXTI_EINT0);

	toggle = ((toggle == TRUE) ? FALSE:TRUE);

#ifdef LPC177x_8x_ADC_BURST_MULTI
	if(toggle)
	{
		ADC_ChannelCmd(LPC_ADC,_ADC_CHANNEL_n,ENABLE);

		//Turn on LED -> indicate that extended channel was enable
		GPIO_ClearValue(LED_PORT, LED_PIN);
	}
	else
	{
		ADC_ChannelCmd(LPC_ADC,_ADC_CHANNEL_n,DISABLE);

		// Turn off LED ->indicate that extended channel was disable
		GPIO_SetValue(LED_PORT, LED_PIN);
	}
#endif
}
#endif
/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief		Print menu
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void print_menu(void)
{
	_DBG(menu1);
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief		c_entry: Main ADC program body
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void c_entry(void)
{
	uint32_t tmp;
	uint32_t adc_value;
	uint8_t  quit;
	EXTI_InitTypeDef EXTICfg;
	
	GPIO_Init();
	
	/* Initialize debug via UART0
	* – 115200bps
	* – 8 data bit
	* – No parity
	* – 1 stop bit
	* – No flow control
	*/
	debug_frmwrk_init();

	// print welcome screen
	print_menu();

	/*
	* Init ADC pin connect
	* AD0.2 on P0.25
	*/
	PINSEL_ConfigPin(BRD_ADC_PREPARED_CH_PORT, BRD_ADC_PREPARED_CH_PIN, BRD_ADC_PREPARED_CH_FUNC_NO);
	PINSEL_SetAnalogPinMode(BRD_ADC_PREPARED_CH_PORT,BRD_ADC_PREPARED_CH_PIN,ENABLE);

#ifdef LPC177x_8x_ADC_BURST_MULTI
	/*
	* Init ADC pin connect
	* AD0.3 on P0.26
	*/
	PINSEL_ConfigPin(0, 26, 1);
    PINSEL_SetAnalogPinMode(0,26,ENABLE);

#endif

	/* Configuration for ADC:
	*  select: ADC channel 2
	*  		ADC channel 3
	*  ADC conversion rate = 400KHz
	*/
	ADC_Init(LPC_ADC, 400000);
	ADC_ChannelCmd(LPC_ADC,BRD_ADC_PREPARED_CHANNEL,ENABLE);

#ifdef LPC177x_8x_ADC_BURST_MULTI
	ADC_ChannelCmd(LPC_ADC,_ADC_CHANNEL_n,ENABLE);
#endif

#ifdef LPC177x_8x_ADC_INJECT_TEST
	//Config P2.10 as EINT0
	PINSEL_ConfigPin(2,10,1);
	EXTI_Init();

	EXTICfg.EXTI_Line = EXTI_EINT0;
	/* edge sensitive */
	EXTICfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	EXTICfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;

	EXTI_Config(&EXTICfg);
	GPIO_SetDir(LED_PORT,LED_PIN,1);
	GPIO_SetValue(LED_PORT,LED_PIN);

	NVIC_EnableIRQ(EINT0_IRQn);
#endif

	//Start burst conversion
	ADC_BurstCmd(LPC_ADC,ENABLE);

	while(1)
	{
		adc_value =  ADC_ChannelGetData(LPC_ADC,BRD_ADC_PREPARED_CHANNEL);
		_DBG("ADC value on channel "); _DBD(BRD_ADC_PREPARED_CHANNEL); _DBG(": ");
		_DBD32(adc_value);
		_DBG_("");

#ifdef LPC177x_8x_ADC_BURST_MULTI
		adc_value =  ADC_ChannelGetData(LPC_ADC,_ADC_CHANNEL_n);
		_DBG("ADC value on channel 3: ");
		_DBD32(adc_value);
		_DBG_("");
#endif
		// Wait for a while
		for(tmp = 0; tmp < 1500000; tmp++);

		if(_DG_NONBLOCK(&quit) &&
			(quit == 'Q' || quit == 'q'))
			break;
	}
    _DBG_("Demo termination!!!");
	ADC_DeInit(LPC_ADC);
	
	GPIO_Deinit();
	
}

/* Support required entry point for other toolchain */
int main (void)
{
	c_entry();
	return 0;
}

/**
 * @}
 */
