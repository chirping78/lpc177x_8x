/***************************************************************************
 **
 **    This file defines the board specific definition
 **
 **    Used with ARM IAR C/C++ Compiler and Assembler.
 **
 **    (c) Copyright IAR Systems 2007
 **
 **    $Revision: 24636 $
 **
 ***************************************************************************/
#ifndef __BOARD_H
#define __BOARD_H

#define KHZ           *1000l
#define HZ            *1l

#define LongToBin(n) (((n >> 21) & 0x80) | \
                      ((n >> 18) & 0x40) | \
                      ((n >> 15) & 0x20) | \
                      ((n >> 12) & 0x10) | \
                      ((n >>  9) & 0x08) | \
                      ((n >>  6) & 0x04) | \
                      ((n >>  3) & 0x02) | \
                      ((n      ) & 0x01))

#define __BIN(n) LongToBin(0x##n##l)

#define BIN32(b1,b2,b3,b4) ((((uint32_t)__BIN(b1)) << 24UL) + \
                            (((uint32_t)__BIN(b2)) << 16UL) + \
                            (((uint32_t)__BIN(b3)) <<  8UL) + \
                              (uint32_t)__BIN(b4))


#define I2C0_INTR_PRIORITY  1
#define DLY_I2C_TIME_OUT    1000


// USB Data Link LED
#define USB_D_LINK_LED_MASK (1UL<<18)
#define USB_D_LINK_LED_DIR  LPC_GPIO1->DIR//IO1DIR
#define USB_D_LINK_LED_FDIR LPC_GPIO1->DIR//FIO1DIR
#define USB_D_LINK_LED_SET  LPC_GPIO1->SET//IO1SET
#define USB_D_LINK_LED_FSET LPC_GPIO1->SET//FIO1SET
#define USB_D_LINK_LED_CLR  LPC_GPIO1->CLR//IO1CLR
#define USB_D_LINK_LED_FCLR LPC_GPIO1->CLR//FIO1CLR
#define USB_D_LINK_LED_IO   LPC_GPIO1->PIN//IO1PIN
#define USB_D_LINK_LED_FIO  LPC_GPIO1->PIN//FIO1PIN

// USB Host Link LED
#define USB_H_LINK_LED_MASK (1UL<<13)
#define USB_H_LINK_LED_DIR  IO1DIR
#define USB_H_LINK_LED_FDIR FIO1DIR
#define USB_H_LINK_LED_SET  IO1SET
#define USB_H_LINK_LED_FSET FIO1SET
#define USB_H_LINK_LED_CLR  IO1CLR
#define USB_H_LINK_LED_FCLR FIO1CLR
#define USB_H_LINK_LED_IO   IO1PIN
#define USB_H_LINK_LED_FIO  FIO1PIN

// Buttons
#define BUT1_MASK           (1UL<<19)
#define BUT1_FDIR           LPC_GPIO2->DIR//FIO2DIR
#define BUT1_FIO            LPC_GPIO2->PIN//FIO2PIN

#define BUT2_MASK           (1UL<<21)
#define BUT2_FDIR           LPC_GPIO2->DIR//FIO2DIR
#define BUT2_FIO            LPC_GPIO2->PIN//FIO2PIN

// MMC/SD card switches
// Card present
#define MMC_CP_MASK         (1UL << 11)
#define MMC_CP_FDIR         LPC_GPIO2->DIR//FIO2DIR
#define MMC_CP_FIO          LPC_GPIO2->PIN//FIO2PIN

// Write protect
#define MMC_WP_MASK         (1UL << 19)
#define MMC_WP_FDIR         LPC_GPIO4->DIR//FIO4DIR
#define MMC_WP_FIO          LPC_GPIO4->PIN//FIO4PIN

// Analog trim
#define ANALOG_TRIM_CHANNEL 7

// VS1002
#define VS1002_CS_AU_MASK   (1UL << 15)
#define VS1002_CS_AU_FDIR   LPC_GPIO4->DIR//FIO4DIR
#define VS1002_CS_AU_FSET   LPC_GPIO4->SET//FIO4SET
#define VS1002_CS_AU_FCLR   LPC_GPIO4->CLR//FIO4CLR
#define VS1002_CS_AU_FIO    LPC_GPIO4->PIN//FIO4PIN

#define VS1002_DREQ_MASK    (1UL << 16)
#define VS1002_DREQ_FDIR    LPC_GPIO4->DIR//FIO4DIR
#define VS1002_DREQ_FIO     LPC_GPIO4->PIN//FIO4PIN

#define VS1002_SS_MASK      (1UL << 23)
#define VS1002_SS_FDIR      LPC_GPIO2->DIR//FIO2DIR
#define VS1002_SS_FIO       LPC_GPIO2->PIN//FIO2PIN

// Touch screen
#define TS_X1_MASK          (1UL << 24)
#define TS_X1_IO            LPC_GPIO0->PIN//IO0PIN
#define TS_X1_FIO           LPC_GPIO0->FIO//FIO0PIN
#define TS_X1_DIR           LPC_GPIO0->DIR//IO0DIR
#define TS_X1_FDIR          LPC_GPIO0->DIR//FIO0DIR
#define TS_X1_SET           LPC_GPIO0->SET//IO0SET
#define TS_X1_CLR           LPC_GPIO0->CLR//IO0CLR
#define TS_X1_FSET          LPC_GPIO0->SET//FIO0SET
#define TS_X1_FCLR          LPC_GPIO0->CLR//FIO0CLR
#define TS_X1_INTR_R        LPC_GPIO0->INTENR//IO0INTENR
#define TS_X1_INTR_CLR      LPC_GPIO0->INTCLR//IO0INTCLR

#endif
