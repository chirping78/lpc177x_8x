/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_glcd.c
 *    Description : Graphical LCD driver
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "board.h"
#include "drv_glcd.h"
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_pinsel.h"
#include "sdram_k4s561632j.h"
#include "Cursor.h"

//#define MHZ           

#define C_GLCD_CLK_PER_LINE     (C_GLCD_H_SIZE + C_GLCD_H_PULSE + C_GLCD_H_FRONT_PORCH + C_GLCD_H_BACK_PORCH)
#define C_GLCD_LINES_PER_FRAME  (C_GLCD_V_SIZE + C_GLCD_V_PULSE + C_GLCD_V_FRONT_PORCH + C_GLCD_V_BACK_PORCH)
#define C_GLCD_PIX_CLK          (6.4*1000000l)


#define LCD_VRAM_BASE_ADDR 	((uint32_t)SDRAM_BASE_ADDR + 0x00100000)
#define LCD_CURSOR_BASE_ADDR 	((uint32_t)0x20088800)

static pFontType_t pCurrFont = NULL;
static LdcPixel_t TextColour;
static LdcPixel_t TextBackgndColour;

static uint32_t TextX_Pos = 0;
static uint32_t TextY_Pos = 0;

static uint32_t XL_Win = 0;
static uint32_t YU_Win = 0;
static uint32_t XR_Win = C_GLCD_H_SIZE-1;
static uint32_t YD_Win = C_GLCD_V_SIZE-1;

static uint32_t TabSize = TEXT_DEF_TAB_SIZE;

static uint32_t WindY_Size, WindX_Size;
static uint32_t CurrY_Size, CurrX_Size;
static uint32_t *pWind;
static uint32_t *pPix;

/*************************************************************************
 * Function Name: GLCD_Cursor_Cnfg
 * Parameters:
 *
 * Return: none
 *
 * Description: Configure the cursor
 *
 *************************************************************************/
void GLCD_Cursor_Cfg(int Cfg)
{
  LPC_LCD->CRSR_CFG = Cfg;
}
/*************************************************************************
 * Function Name: GLCD_Cursor_En
 * Parameters: cursor - Cursor Number
 *
 * Return: none
 *
 * Description: Enable Cursor
 *
 *************************************************************************/
void GLCD_Cursor_En(int cursor)
{
  LPC_LCD->CRSR_CTRL |= (cursor<<4);
  LPC_LCD->CRSR_CTRL |= 1;
}

/*************************************************************************
 * Function Name: GLCD_Cursor_Dis
 * Parameters: None
 *
 * Return: none
 *
 * Description: Disable Cursor
 *
 *************************************************************************/
void GLCD_Cursor_Dis(int cursor)
{
  LPC_LCD->CRSR_CTRL &= (1<<0);
}

/*************************************************************************
 * Function Name: GLCD_Move_Cursor
 * Parameters: x - cursor x position
 *             y - cursor y position
 *
 * Return: none
 *
 * Description: Moves cursor on position (x,y). Negativ values are posible.
 *
 *************************************************************************/
void GLCD_Move_Cursor(int x, int y)
{
  LPC_LCD->CRSR_CLIP = 0;
  LPC_LCD->CRSR_XY = 0;
  if(0 <= x)
  {//no clipping
    LPC_LCD->CRSR_XY |= (x & 0x3FF);
  }
  else
  {//clip x
    LPC_LCD->CRSR_CLIP |= -x;
  }

  if(0 <= y)
  {//no clipping

    LPC_LCD->CRSR_XY |= (y << 16);
  }
  else
  {//clip y
    LPC_LCD->CRSR_CLIP |= (-y << 8);
  }
}

/*************************************************************************
 * Function Name: GLCD_Copy_Cursor
 * Parameters: pCursor - pointer to cursor conts image
 *             cursor - cursor Number (0,1,2 or 3)
 *                      for 64x64(size 256) pix cursor always use 0
 *             size - cursor size in words
 * Return: none
 *
 * Description: Copy Cursor from const image to LCD RAM image
 *
 *************************************************************************/
void GLCD_Copy_Cursor (const uint32_t *pCursor, int cursor, int size)
{
   	uint32_t i ;
   	uint32_t * pDst = (uint32_t *)LCD_CURSOR_BASE_ADDR;
   
   	pDst += cursor*64;

   	for(i = 0; i < size ; i++) 
//	   *pDst++ = *pCursor++; 
	{
		*pDst = *pCursor;
		pDst++;
		pCursor++;
	}
}
/*************************************************************************
 * Function Name: GLCD_Init
 * Parameters: const uint32_t *pPain, const uint32_t * pPallete
 *
 * Return: none
 *
 * Description: GLCD controller init
 *
 *************************************************************************/
void GLCD_Init (const uint32_t *pPain, const uint32_t * pPallete)
{
	uint32_t i;
	uint32_t *pDst = (uint32_t *)LCD_VRAM_BASE_ADDR;
	// Assign pins
	LPC_IOCON->P0_4 = 0x27;
	LPC_IOCON->P0_5 = 0x27;
	LPC_IOCON->P0_6 = 0x27;
	LPC_IOCON->P0_7 = 0x27;
	LPC_IOCON->P0_8 = 0x27;
	LPC_IOCON->P0_9 = 0x27;
	LPC_IOCON->P1_20 = 0x27;
	LPC_IOCON->P1_21 = 0x27;
	LPC_IOCON->P1_22 = 0x27;
	LPC_IOCON->P1_23 = 0x27;
	LPC_IOCON->P1_24 = 0x27;
	LPC_IOCON->P1_25 = 0x27;
	LPC_IOCON->P1_26 = 0x27;
	LPC_IOCON->P1_27 = 0x27;
	LPC_IOCON->P1_28 = 0x27;
	LPC_IOCON->P1_29 = 0x27;

	LPC_IOCON->P2_1 = 0x20;
	LPC_IOCON->P2_2 = 0x27;
	LPC_IOCON->P2_3 = 0x27;
	LPC_IOCON->P2_4 = 0x27;
	LPC_IOCON->P2_5 = 0x27;
	LPC_IOCON->P2_6 = 0x27;
	LPC_IOCON->P2_7 = 0x27;
	LPC_IOCON->P2_8 = 0x27;
	LPC_IOCON->P2_9 = 0x27;

	LPC_IOCON->P2_12 = 0x27;
	LPC_IOCON->P2_13 = 0x27;
	LPC_IOCON->P4_28 = 0x27;
	LPC_IOCON->P4_29 = 0x27;
	
	/*Back light enable*/
	LPC_GPIO2->DIR = (1<<1);
	LPC_GPIO2->SET= (1<<1);

	//Turn on LCD clock
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCLCD, ENABLE);
	
	// Disable cursor
	LPC_LCD->CRSR_CTRL &=~(1<<0);
	
	// disable GLCD controller	
	LPC_LCD->CTRL = 0;
	// 24 bpp
	LPC_LCD->CTRL &= ~(0x07 <<1);
	LPC_LCD->CTRL |=(5<<1);
	
	// TFT panel
	LPC_LCD->CTRL |= (1<<5);
	// single panel
	LPC_LCD->CTRL &= ~(1<<7);
	// notmal output
	LPC_LCD->CTRL &= ~(1<<8);
	// little endian byte order
	LPC_LCD->CTRL &= ~(1<<9);
	// little endian pix order
	LPC_LCD->CTRL &= ~(1<<10);
	// disable power
	LPC_LCD->CTRL &= ~(1<<11);
	// init pixel clock
	LPC_SC->LCD_CFG = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER) / ((uint32_t)C_GLCD_PIX_CLK);
	// bypass inrenal clk divider
	LPC_LCD->POL |=(1<<26);
	// clock source for the LCD block is HCLK
	LPC_LCD->POL &= ~(1<<5);
	// LCDFP pin is active LOW and inactive HIGH
	LPC_LCD->POL |= (1<<11);
	// LCDLP pin is active LOW and inactive HIGH
	LPC_LCD->POL |= (1<<12);
	// data is driven out into the LCD on the falling edge
	LPC_LCD->POL |= (1<<13);
	// active high
	LPC_LCD->POL &= ~(1<<14);
	LPC_LCD->POL &= ~(0x3FF <<16);
	LPC_LCD->POL |= (C_GLCD_H_SIZE-1)<<16;
	
	// init Horizontal Timing
	LPC_LCD->TIMH = 0; //reset TIMH before set value
	LPC_LCD->TIMH |= (C_GLCD_H_BACK_PORCH - 1)<<24;
	LPC_LCD->TIMH |= (C_GLCD_H_FRONT_PORCH - 1)<<16;
	LPC_LCD->TIMH |= (C_GLCD_H_PULSE - 1)<<8;
	LPC_LCD->TIMH |= ((C_GLCD_H_SIZE/16) - 1)<<2;
	
	// init Vertical Timing
	LPC_LCD->TIMV = 0;  //reset TIMV value before setting
	LPC_LCD->TIMV |= (C_GLCD_V_BACK_PORCH)<<24;
	LPC_LCD->TIMV |= (C_GLCD_V_FRONT_PORCH)<<16;
	LPC_LCD->TIMV |= (C_GLCD_V_PULSE - 1)<<10;
	LPC_LCD->TIMV |= C_GLCD_V_SIZE - 1;
	// Frame Base Address doubleword aligned
	LPC_LCD->UPBASE = LCD_VRAM_BASE_ADDR & ~7UL ;
	LPC_LCD->LPBASE = LCD_VRAM_BASE_ADDR & ~7UL ;
        // init colour pallet

	if(NULL != pPallete)
	{
		GLCD_SetPallet(pPallete);
	}
	
	if (NULL == pPain)
	{
		// clear display memory
		for( i = 0; (C_GLCD_H_SIZE * C_GLCD_V_SIZE) > i; i++)
		{
	  		*pDst++ = 0;
		}
	}
	else
	{
		// set display memory
		for(i = 0; (C_GLCD_H_SIZE * C_GLCD_V_SIZE) > i; i++)
		{
	  		*pDst++ = *pPain++;
		}
	}

  	for(i = C_GLCD_ENA_DIS_DLY; i; i--);
}

/*************************************************************************
 * Function Name: GLCD_SetPallet
 * Parameters: const uint32_t * pPallete
 *
 * Return: none
 *
 * Description: GLCD init colour pallete
 *
 *************************************************************************/
void GLCD_SetPallet (const uint32_t * pPallete)
{
	uint32_t i;
	uint32_t * pDst = (uint32_t *)LPC_LCD->PAL;
	assert(pPallete);
	for (i = 0; i < 128; i++)
	{
	*pDst++ = *pPallete++;
	}
}

/*************************************************************************
 * Function Name: GLCD_Ctrl
 * Parameters: Bool bEna
 *
 * Return: none
 *
 * Description: GLCD enable disabe sequence
 *
 *************************************************************************/
void GLCD_Ctrl (Bool bEna)
{
	volatile uint32_t i;
  if (bEna)
  {
//    LCD_CTRL_bit.LcdEn = 1;
    LPC_LCD->CTRL |= (1<<0);
    for(i = C_GLCD_PWR_ENA_DIS_DLY; i; i--);
//    LCD_CTRL_bit.LcdPwr= 1;   // enable power
    LPC_LCD->CTRL |= (1<<11);
  }
  else
  {
//    LCD_CTRL_bit.LcdPwr= 0;   // disable power
    LPC_LCD->CTRL &= ~(1<<11);
    for(i = C_GLCD_PWR_ENA_DIS_DLY; i; i--);
//    LCD_CTRL_bit.LcdEn = 0;
    LPC_LCD->CTRL &= ~(1<<0);
  }
}

/*************************************************************************
 * Function Name: GLCD_SetFont
 * Parameters: pFontType_t pFont, LdcPixel_t Color
 *              LdcPixel_t BackgndColor
 *
 * Return: none
 *
 * Description: Set current font, font color and background color
 *
 *************************************************************************/
void GLCD_SetFont(pFontType_t pFont, LdcPixel_t Color, LdcPixel_t BackgndColor)
{
  pCurrFont = pFont;
  TextColour = Color;
  TextBackgndColour = BackgndColor;
}

/*************************************************************************
 * Function Name: GLCD_SetWindow
 * Parameters: uint32_t X_Left, uint32_t Y_Up,
 *             uint32_t X_Right, uint32_t Y_Down
 *
 * Return: none
 *
 * Description: Set draw window XY coordinate in pixels
 *
 *************************************************************************/
void GLCD_SetWindow(uint32_t X_Left, uint32_t Y_Up,
                    uint32_t X_Right, uint32_t Y_Down)
{
  assert(X_Right < C_GLCD_H_SIZE);
  assert(Y_Down < C_GLCD_V_SIZE);
  assert(X_Left < X_Right);
  assert(Y_Up < Y_Down);
  XL_Win = X_Left;
  YU_Win = Y_Up;
  XR_Win = X_Right;
  YD_Win = Y_Down;
}

/*************************************************************************
 * Function Name: GLCD_TextSetPos
 * Parameters: uint32_t X_UpLeft, uint32_t Y_UpLeft,
 *             uint32_t X_DownLeft, uint32_t Y_DownLeft
 *
 * Return: none
 *
 * Description: Set text X,Y coordinate in characters
 *
 *************************************************************************/
void GLCD_TextSetPos(uint32_t X, uint32_t Y)
{
  TextX_Pos = X;
  TextY_Pos = Y;
}

/*************************************************************************
 * Function Name: GLCD_TextSetTabSize
 * Parameters: uint32_t Size
 *
 * Return: none
 *
 * Description: Set text tab size in characters
 *
 *************************************************************************/
void GLCD_TextSetTabSize(uint32_t Size)
{
  TabSize = Size;
}

/*************************************************************************
 * Function Name: LCD_SET_WINDOW
 * Parameters: int c
 *
 * Return: none
 *
 * Description: Put char function
 *
 *************************************************************************/
static
void LCD_SET_WINDOW (uint32_t X_Left, uint32_t X_Right,
                     uint32_t Y_Up, uint32_t Y_Down)
{
  pPix = pWind = ((uint32_t *)LCD_VRAM_BASE_ADDR) + X_Left + (Y_Up*C_GLCD_H_SIZE);
  WindX_Size = X_Right - X_Left;
  WindY_Size = Y_Down - Y_Up;
  CurrX_Size = CurrY_Size = 0;
}

/*************************************************************************
 * Function Name: LCD_SET_WINDOW
 * Parameters: int c
 *
 * Return: none
 *
 * Description: Put char function
 *
 *************************************************************************/
static
void LCD_WRITE_PIXEL (uint32_t Pixel)
{
  *pPix++ = Pixel;
  if (++CurrX_Size > WindX_Size)
  {
    CurrX_Size = 0;
    if(++CurrY_Size > WindY_Size)
    {
      CurrY_Size = 0;
    }
    pPix = pWind + CurrY_Size * C_GLCD_H_SIZE;
  }
}

/*************************************************************************
 * Function Name: GLCD_TextCalcWindow
 * Parameters: uint32_t * pXL, uint32_t * pXR,
 *             uint32_t * pYU, uint32_t * pYD,
 *             uint32_t * pH_Size, uint32_t * pV_Size
 *
 * Return: Bool
 *          FALSE - out of window coordinate aren't valid
 *          TRUE  - the returned coordinate are valid
 *
 * Description: Calculate character window
 *
 *************************************************************************/
static
Bool GLCD_TextCalcWindow (uint32_t * pXL, uint32_t * pXR,
                             uint32_t * pYU, uint32_t * pYD,
                             uint32_t * pH_Size, uint32_t * pV_Size)
{
  *pH_Size = pCurrFont->H_Size;
  *pV_Size = pCurrFont->V_Size;
  *pXL = XL_Win + (TextX_Pos*pCurrFont->H_Size);
  if(*pXL > XR_Win)
  {
    return(FALSE);
  }
  *pYU = YU_Win + (TextY_Pos*pCurrFont->V_Size);
  if(*pYU > YD_Win)
  {
    return(FALSE);
  }

  *pXR   = XL_Win + ((TextX_Pos+1)*pCurrFont->H_Size) - 1;
  if(*pXR > XR_Win)
  {
  	*pH_Size -= *pXR - XR_Win;
    *pXR = XR_Win;
  }

  *pYD = YU_Win + ((TextY_Pos+1)*pCurrFont->V_Size) - 1;
  if(*pYD > YD_Win)
  {
    *pV_Size -= *pYD - YD_Win;
    *pYD = YD_Win;
  }

  return(TRUE);
}

/*************************************************************************
 * Function Name: putchar
 * Parameters: int c
 *
 * Return: none
 *
 * Description: Put char function
 *
 *************************************************************************/
int _putchar (int c)
{
uint8_t *pSrc;
uint32_t H_Line;
uint32_t xl,xr,yu,yd,Temp,V_Size, H_Size, SrcInc = 1;
uint32_t WhiteSpaceNumb;
uint32_t i, j, k;
  if(pCurrFont == NULL)
  {
    return(EOF);
  }
  H_Line = (pCurrFont->H_Size / 8) + ((pCurrFont->H_Size % 8)?1:0);
  switch(c)
  {
  case '\n':  // go to begin of next line (NewLine)
    ++TextY_Pos;
    break;
  case '\r':  // go to begin of this line (Carriage Return)
  	// clear from current position to end of line
  	while(GLCD_TextCalcWindow(&xl,&xr,&yu,&yd,&H_Size,&V_Size))
  	{
      LCD_SET_WINDOW(xl,xr,yu,yd);
	    for(i = 0; i < V_Size; ++i)
	    {
	      for(j = 0; j < H_Size; ++j)
	      {
	        LCD_WRITE_PIXEL(TextBackgndColour);
	      }
	    }
  		++TextX_Pos;
  	}
    TextX_Pos = 0;
    break;
  case '\b': // go back one position (BackSpace)
    if(TextX_Pos)
    {
      --TextX_Pos;
      // del current position
	  	if(GLCD_TextCalcWindow(&xl,&xr,&yu,&yd,&H_Size,&V_Size))
	  	{
        LCD_SET_WINDOW(xl,xr,yu,yd);
		    for(i = 0; i < V_Size; ++i)
		    {
		      for(j = 0; j < H_Size; ++j)
		      {
		        LCD_WRITE_PIXEL(TextBackgndColour);
		      }
		    }
	  	}
    }
    break;
  case '\t':  // go to next Horizontal Tab stop
  	WhiteSpaceNumb = TabSize - (TextX_Pos%TabSize);
  	for(k = 0; k < WhiteSpaceNumb; ++k)
  	{
      LCD_SET_WINDOW(xl,xr,yu,yd);
	  	if(GLCD_TextCalcWindow(&xl,&xr,&yu,&yd,&H_Size,&V_Size))
	  	{
		    for(i = 0; i < V_Size; ++i)
		    {
		      for(j = 0; j < H_Size; ++j)
		      {
		        LCD_WRITE_PIXEL(TextBackgndColour);
		      }
		    }
		    ++TextX_Pos;
	  	}
	  	else
	  	{
	  		break;
	  	}
  	}
    break;
  case '\f':  // go to top of page (Form Feed)
  	// clear entire window
  	H_Size = XR_Win - XL_Win;
  	V_Size = YD_Win - YU_Win;
    // set character window X left, Y right
    LCD_SET_WINDOW(XL_Win,XR_Win,YU_Win,YD_Win);
    // Fill window with background font color
    for(i = 0; i <= V_Size; ++i)
    {
      for(j = 0; j <= H_Size; ++j)
      {
        LCD_WRITE_PIXEL(TextBackgndColour);
      }
    }

  	TextX_Pos = TextY_Pos = 0;
    break;
  case '\a':  // signal an alert (BELl)
    TEXT_BEL1_FUNC();
    break;
  default:
    // Calculate the current character base address from stream
    // and the character position
    if((c <  pCurrFont->CharacterOffset) &&
    	 (c >= pCurrFont->CharactersNuber))
   	{
   		c = 0;
    }
    else
    {
    	c -= pCurrFont->CharacterOffset;
    }
    pSrc = pCurrFont->pFontStream + (H_Line * pCurrFont->V_Size * c);
    // Calculate character window and fit it in the text window
    if(GLCD_TextCalcWindow(&xl,&xr,&yu,&yd,&H_Size,&V_Size))
    {
	    // set character window X left, Y right
	    LCD_SET_WINDOW(xl,xr,yu,yd);
	    // Send char data
	    for(i = 0; i < V_Size; ++i)
	    {
        SrcInc = H_Line;
        for(j = 0; j < H_Size; ++j)
	      {
	        Temp = (*pSrc & (1UL << (j&0x7)))?TextColour:TextBackgndColour;
	        LCD_WRITE_PIXEL(Temp);
	        if((j&0x7) == 7)
	        {
	          ++pSrc;
            --SrcInc;
	        }
	      }
        // next line of character
	      pSrc += SrcInc;
	    }
    }
    ++TextX_Pos;
  }
  return(c);
}

/*************************************************************************
 * Function Name: GLCD_LoadPic
 * Parameters: uint32_t X_Left, uint32_t Y_Up, Bmp_t * pBmp
 *
 * Return: none
 *
 * Description: Load picture in VRAM memory area
 *
 *************************************************************************/
void GLCD_LoadPic (uint32_t X_Left, uint32_t Y_Up, Bmp_t * pBmp, uint32_t Mask)
{
uint32_t i, j;
uint32_t * pData = ((uint32_t *) LCD_VRAM_BASE_ADDR) + X_Left + (Y_Up * C_GLCD_H_SIZE);
uint32_t * pSrc = pBmp->pPicStream;
uint32_t X_LeftHold;
  for(i = 0; i < pBmp->V_Size; i++)
  {
    if(Y_Up++ >= C_GLCD_V_SIZE)
    {
      break;
    }
    for(j = 0; j < pBmp->H_Size; j++)
    {
      if(X_LeftHold++ >= C_GLCD_H_SIZE)
      {
        pSrc += pBmp->H_Size - j;
        break;
      }
      *(pData+j) = *pSrc++ ^ Mask;
    }
    X_LeftHold = X_Left;
    pData += C_GLCD_H_SIZE;
  }
}

