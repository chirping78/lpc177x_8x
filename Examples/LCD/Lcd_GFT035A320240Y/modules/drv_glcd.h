/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_glcd.h
 *    Description : Graphical LCD driver include file
 *
 *    History :
 *    1. Date        : 6, March 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *
 *    $Revision: 24636 $
 *
 *    @Modify: NXP MCU Application Team - NguyenCao
 *    @Date: 04. March. 2011
 **************************************************************************/
#include "lpc_types.h"

#ifndef __GLCD_DRV_H
#define __GLCD_DRV_H

/**
 * @brief A struct for Bitmap on LCD screen
 */
typedef struct _Bmp_t 
{
  uint32_t  H_Size;
  uint32_t  V_Size;
  uint32_t  BitsPP;
  uint32_t  BytesPP;
  uint32_t *pPalette;
  uint32_t *pPicStream;
  uint8_t *pPicDesc;
} Bmp_t, *pBmp_t;

/**
 * @brief A struct for Font Type on LCD screen
 */
 
typedef struct _FontType_t 
{
  uint32_t H_Size;
  uint32_t V_Size;
  uint32_t CharacterOffset;
  uint32_t CharactersNuber;
  uint8_t *pFontStream;
  uint8_t *pFontDesc;
} FontType_t, *pFontType_t;

typedef uint32_t LdcPixel_t, *pLdcPixel_t;

#define C_GLCD_REFRESH_FREQ     (50HZ)
#define C_GLCD_H_SIZE           320
#define C_GLCD_H_PULSE          30
#define C_GLCD_H_FRONT_PORCH    20
#define C_GLCD_H_BACK_PORCH     38
#define C_GLCD_V_SIZE           240
#define C_GLCD_V_PULSE          3
#define C_GLCD_V_FRONT_PORCH    5
#define C_GLCD_V_BACK_PORCH     15

#define C_GLCD_PWR_ENA_DIS_DLY  10000
#define C_GLCD_ENA_DIS_DLY      10000

#define CRSR_PIX_32     0
#define CRSR_PIX_64     1
#define CRSR_ASYNC      0
#define CRSR_FRAME_SYNC 2

#define TEXT_DEF_TAB_SIZE 5

#define TEXT_BEL1_FUNC()

void GLCD_Init (const uint32_t *pPain, const uint32_t * pPallete);
void GLCD_SetPallet (const uint32_t * pPallete);
void GLCD_Ctrl (Bool bEna);
void GLCD_Cursor_Cfg(int Cfg);
void GLCD_Cursor_En(int cursor);
void GLCD_Cursor_Dis(int cursor);
void GLCD_Move_Cursor(int x, int y);
void GLCD_Copy_Cursor (const uint32_t *pCursor, int cursor, int size);
void GLCD_SetFont(pFontType_t pFont, LdcPixel_t Color, LdcPixel_t BackgndColor);
void GLCD_SetWindow(uint32_t X_Left, uint32_t Y_Up,
                    uint32_t X_Right, uint32_t Y_Down);
void GLCD_TextSetPos(uint32_t X, uint32_t Y);
void GLCD_TextSetTabSize(uint32_t Size);
static void LCD_SET_WINDOW (uint32_t X_Left, uint32_t X_Right,
                            uint32_t Y_Up, uint32_t Y_Down);
static void LCD_WRITE_PIXEL (uint32_t Pixel);
static Bool GLCD_TextCalcWindow (uint32_t * pXL, uint32_t * pXR,
                                    uint32_t * pYU, uint32_t * pYD,
                                    uint32_t * pH_Size, uint32_t * pV_Size);
void GLCD_LoadPic (uint32_t X_Left, uint32_t Y_Up, Bmp_t * pBmp, uint32_t Mask);
int _putchar (int c);


#endif // __GLCD_DRV_H
