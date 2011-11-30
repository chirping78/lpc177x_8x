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
#include "sdram_k4s561632j.h"
#include "logo.h"
#include "Cursor.h"
#include "sensor_smb380.h"
#include "lpc_types.h"
#include "system_LPC177x_8x.h"
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_timer.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_lcd.h"
#include "lpc177x_8x_pwm.h"
#include "lpc177x_8x_adc.h"
#include "bsp.h"


/** @defgroup LCD_GFT035A320240Y	LCD GFT035A320240Y
 * @ingroup LCD_Examples
 * @{
 */

#define LCD_VRAM_BASE_ADDR_UPPER 	((uint32_t)SDRAM_BASE_ADDR + 0x00100000)
#define LCD_VRAM_BASE_ADDR_LOWER 	(LCD_VRAM_BASE_ADDR_UPPER + 1024*768*4)
#define LCD_CURSOR_BASE_ADDR 	((uint32_t)0x20088800)

#define TIMER0_TICK_PER_SEC   10
#define BEEP 	 (1 << 30)
#define LCD_REFRESH_FREQ     (50HZ)
#define LCD_H_SIZE           320
#define LCD_H_PULSE          30
#define LCD_H_FRONT_PORCH    20
#define LCD_H_BACK_PORCH     38
#define LCD_V_SIZE           240
#define LCD_V_PULSE          3
#define LCD_V_FRONT_PORCH    5
#define LCD_V_BACK_PORCH     15
#define LCD_CLK_PER_LINE     (LCD_H_SIZE + LCD_H_PULSE + LCD_H_FRONT_PORCH + LCD_H_BACK_PORCH)
#define LCD_LINES_PER_FRAME  (LCD_V_SIZE + LCD_V_PULSE + LCD_V_FRONT_PORCH + LCD_V_BACK_PORCH)
#define LCD_PIX_CLK          (6.5*1000000l)
#define LCD_PWR_ENA_DIS_DLY  10000
#define LCD_ENA_DIS_DLY      10000

#define CRSR_PIX_32     0
#define CRSR_PIX_64     1
#define CRSR_ASYNC      0
#define CRSR_FRAME_SYNC 2

#define TEXT_DEF_TAB_SIZE 5

#define _PWM_NO_USED    1
#define _PWM_CHANNEL_NO 2

#define _BACK_LIGHT_BASE_CLK (1000/2)

uint8_t Smb380Id, Smb380Ver;

extern uint8_t * LogoStream;
#if (LOGO_BPP == 2)
extern uint8_t * LogoPalette;
#endif

Bmp_t LogoPic =
{
  320,
  240,
  LOGO_BPP,
  BMP_BYTES_PP,
  #if (LOGO_BPP == 2)
  (uint8_t *)&LogoPalette,
  #else
  NULL,
  #endif
  (uint8_t *)&LogoStream,
  ( uint8_t *)"Logos picture"
};


void DelayNS(uint32_t dly)
{
	uint32_t i = 0;

	for ( ; dly > 0; dly--)
		for (i = 0; i < 5000; i++);
}

/*************************************************************************
 * Function Name: Set LCD backlight
 * Parameters: level     Backlight value
 *
 * Return: none
 *
 * Description: None
 *
 *************************************************************************/
void SetBackLight(uint32_t level)
{
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  PWM_MatchUpdate(_PWM_NO_USED, _PWM_CHANNEL_NO, level, PWM_MATCH_UPDATE_NOW);
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = _PWM_CHANNEL_NO;
  PWMMatchCfgDat.ResetOnMatch = DISABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch(_PWM_NO_USED, &PWMMatchCfgDat);


  /* Enable PWM Channel Output */
  PWM_ChannelCmd(_PWM_NO_USED, _PWM_CHANNEL_NO, ENABLE);

  /* Reset and Start counter */
  PWM_ResetCounter(_PWM_NO_USED);

  PWM_CounterCmd(_PWM_NO_USED, ENABLE);

  /* Start PWM now */
  PWM_Cmd(_PWM_NO_USED, ENABLE);
}

/*************************************************************************
 * Function Name: Get backlight value from user
 * Parameters: none
 *
 * Return: none
 *
 * Description: backlight value
 *
 *************************************************************************/
uint32_t GetBacklightVal (void) {
  uint32_t val;
  uint32_t backlight_off, pclk;

  ADC_StartCmd(LPC_ADC, ADC_START_NOW);

  while (!(ADC_ChannelGetStatus(LPC_ADC, BRD_ADC_PREPARED_CHANNEL, ADC_DATA_DONE)));

  val = ADC_ChannelGetData(LPC_ADC, BRD_ADC_PREPARED_CHANNEL);

  val = (val >> 7) & 0x3F;

  pclk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
  backlight_off = pclk/(_BACK_LIGHT_BASE_CLK*20);
  val =  val* (pclk*9/(_BACK_LIGHT_BASE_CLK*20))/0x3F;

  return backlight_off + val;
}


/*************************************************************************
 * Function Name: c_entry
 * Parameters: none
 *
 * Return: none
 *
 * Description: entry
 *
 *************************************************************************/
  void c_entry(void)
{
  typedef uint32_t ram_unit;
  uint32_t i, pclk;
  uint8_t red, green, blue;
  LcdPixel_t color;
  uint32_t xs, ys;
  uint32_t xe, ye, weight;
  SMB380_Data_t XYZT;
  uint32_t backlight;
  PWM_TIMERCFG_Type PWMCfgDat;
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  LCD_Cursor_Config_Type cursor_config;
  LCD_Config_Type lcd_config;
  int cursor_x = (LCD_H_SIZE - CURSOR_H_SIZE)/2, cursor_y = (LCD_V_SIZE - CURSOR_V_SIZE)/2;

  /***************/
  /* Initialize PWM */
  /***************/
  PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
  PWMCfgDat.PrescaleValue = 1;
  PWM_Init(_PWM_NO_USED, PWM_MODE_TIMER, (void *) &PWMCfgDat);

  PINSEL_ConfigPin (2, 1, 1);
  PWM_ChannelConfig(_PWM_NO_USED, _PWM_CHANNEL_NO, PWM_CHANNEL_SINGLE_EDGE);

  // Set MR0
  pclk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
  PWM_MatchUpdate(_PWM_NO_USED, 0,pclk/_BACK_LIGHT_BASE_CLK, PWM_MATCH_UPDATE_NOW);
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = 0;
  PWMMatchCfgDat.ResetOnMatch = ENABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch(_PWM_NO_USED, &PWMMatchCfgDat);

  /***************/
  /** Initialize ADC */
  /***************/
  PINSEL_ConfigPin (BRD_ADC_PREPARED_CH_PORT,
  					BRD_ADC_PREPARED_CH_PIN,
  					BRD_ADC_PREPARED_CH_FUNC_NO);
  PINSEL_SetAnalogPinMode(BRD_ADC_PREPARED_CH_PORT,BRD_ADC_PREPARED_CH_PIN,ENABLE);

  ADC_Init(LPC_ADC, 400000);
  ADC_IntConfig(LPC_ADC, BRD_ADC_PREPARED_INTR, DISABLE);
  ADC_ChannelCmd(LPC_ADC, BRD_ADC_PREPARED_CHANNEL, ENABLE);

  /***************/
  /** Initialize LCD */
  /***************/
  LCD_Enable (FALSE);
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
  // SDRAM Init	= check right board to avoid linking error
  SDRAMInit();
#endif

  lcd_config.big_endian_byte = 0;
  lcd_config.big_endian_pixel = 0;
  lcd_config.hConfig.hbp = LCD_H_BACK_PORCH;
  lcd_config.hConfig.hfp = LCD_H_FRONT_PORCH;
  lcd_config.hConfig.hsw = LCD_H_PULSE;
  lcd_config.hConfig.ppl = LCD_H_SIZE;
  lcd_config.vConfig.lpp = LCD_V_SIZE;
  lcd_config.vConfig.vbp = LCD_V_BACK_PORCH;
  lcd_config.vConfig.vfp = LCD_V_FRONT_PORCH;
  lcd_config.vConfig.vsw = LCD_V_PULSE;
  lcd_config.panel_clk   = LCD_PIX_CLK;
  lcd_config.polarity.active_high = 1;
  lcd_config.polarity.cpl = LCD_H_SIZE;
  lcd_config.lcd_panel_upper =  LCD_VRAM_BASE_ADDR_UPPER;
  lcd_config.lcd_panel_lower =  LCD_VRAM_BASE_ADDR_LOWER;
  #if (LOGO_BPP == 24)
  lcd_config.lcd_bpp = LCD_BPP_24;
  #elif (LOGO_BPP == 16)
  lcd_config.lcd_bpp = LCD_BPP_16;
  #elif (LOGO_BPP == 2)
  lcd_config.lcd_bpp = LCD_BPP_2;
  #else
  while(1);
  #endif
  lcd_config.lcd_type = LCD_TFT;
  lcd_config.lcd_palette = LogoPic.pPalette;

  LCD_Init (&lcd_config);

  //LCD_SetImage(LCD_PANEL_UPPER, LogoPic.pPicStream);
  LCD_SetImage(LCD_PANEL_UPPER, NULL);
  LCD_SetImage(LCD_PANEL_LOWER, NULL);
  xs = (LCD_H_SIZE - LogoPic.H_Size)/2;
  ys = (LCD_V_SIZE - LogoPic.V_Size)/2;
  LCD_LoadPic(LCD_PANEL_UPPER,xs,ys,&LogoPic,0x00);

  // Set backlight
  backlight = GetBacklightVal();
  SetBackLight(backlight);

  // Enable LCD
  LCD_Enable (TRUE);

  // Draw a border
  red = 51;green = 153;blue = 102;
  weight = 2;
  xs = 0;
  ys = 0;
  xe = LCD_H_SIZE - (xs + weight) - 1;
  ye = LCD_V_SIZE - (ys + weight) - 1;
  #if (LOGO_BPP == 24)
  color = (blue<<16|green<<8|red);
  #elif (LOGO_BPP == 16)
  color = (blue << 10 | green << 5 | red);
  #else
  color = 0;
  #endif
  LCD_FillRect (LCD_PANEL_UPPER,xs, xe + weight, ys, ys + weight, color);
  LCD_FillRect (LCD_PANEL_UPPER,xs, xe + weight, ye, ye + weight, color);
  LCD_FillRect (LCD_PANEL_UPPER,xs, xs + weight, ys, ye + weight, color);
  LCD_FillRect (LCD_PANEL_UPPER,xe, xe + weight, ys, ye + weight, color);

  // Draw cursor
  LCD_Cursor_Enable(DISABLE, 0);

  cursor_config.baseaddress = LCD_CURSOR_BASE_ADDR;
  cursor_config.framesync = 1;
  cursor_config.size32 = 0;
  LCD_Cursor_Cfg(&cursor_config);
  LCD_Cursor_SetImage((uint32_t *)Cursor, 0, sizeof(Cursor)/sizeof(uint32_t)) ;

  LCD_Move_Cursor(cursor_x, cursor_y);

  LCD_Cursor_Enable(ENABLE, 0);

  /* Initialize accel?ation sensor */
  SMB380_Init();

  SMB380_GetID(&Smb380Id, &Smb380Ver);

  while(1)
  {
    for(i = 0; i < 100000;  i++);

	backlight = GetBacklightVal();
    SetBackLight(backlight);

    SMB380_GetData (&XYZT);

    cursor_x += XYZT.AccX/512;
    cursor_y += XYZT.AccY/512;

    if((LCD_H_SIZE - CURSOR_H_SIZE/2) < cursor_x)
    {
      cursor_x = LCD_H_SIZE - CURSOR_H_SIZE/2;
    }

    if(-(CURSOR_H_SIZE/2) > cursor_x)
    {
      cursor_x = -(CURSOR_H_SIZE/2);
    }

    if((LCD_V_SIZE - CURSOR_V_SIZE/2) < cursor_y)
    {
      cursor_y = (LCD_V_SIZE - CURSOR_V_SIZE/2);
    }

    if(-(CURSOR_V_SIZE/2) > cursor_y)
    {
      cursor_y = -(CURSOR_V_SIZE/2);
    }

    LCD_Move_Cursor(cursor_x, cursor_y);
  }
}

int main (void)
{
   c_entry();
   return 0;
}

/**
 * @}
 */
