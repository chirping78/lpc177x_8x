/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : i2c1_drv.c
 *    Description : I2C1 driver include file
 *
 *    History :
 *    1. Date        : 13, February 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *
 *    $Revision: 22899 $
 *
 *    @Modify: NXP MCU Application Team - NguyenCao
 *    @Date: 04. March. 2011
 **************************************************************************/
#include "LPC177x_8x.h"

#ifndef __I2C0_DRV_H
#define __I2C0_DRV_H

#define I2C_MAXSPEED            400000  // I2C max speed: 400k/s
#define I2C_SPEED               400000
// I2CONSET Register Bit Definitions
#define I2CON_I2EN  (1 << 6)            // I2C Enable  (0x40)
#define I2CON_STA   (1 << 5)            // START flag  (0x20)
#define I2CON_STO   (1 << 4)            // STOP flag   (0x10)
#define I2CON_SI    (1 << 3)            // I2C Interrupt flag (0x08)
#define I2CON_AA    (1 << 2)            // Assert Acknowlege flag (0x04)

// I2CONCLR Register Bit Definitions
#define I2CON_I2ENC (1 << 6)            // I2C Enable Clear  (0x40)
#define I2CON_STAC  (1 << 5)            // START Clear  (0x20)
#define I2CON_SIC   (1 << 3)            // I2C Interrupt Clear  (0x08)
#define I2CON_AAC   (1 << 2)            // Assert Acknowlege Clear (0x04)

/* Status Errors */
#define I2C_OK 			            0       // transfer ended No Errors
#define I2C_IDLE		            1       // bus idle
#define I2C_BUSY 		            2       // transfer busy
#define I2C_ERROR 		          3       // err: general error
#define I2C_NO_DATA 		        4       // err: No data in block
#define I2C_NACK_ON_DATA 	      5       // err: No ack on data
#define I2C_NACK_ON_ADDRESS 	  6       // err: No ack on address
#define I2C_DEVICE_NOT_PRESENT 	7       // err: Device not present
#define I2C_ARBITRATION_LOST 	  8       // err: Arbitration lost
#define I2C_TIME_OUT 		        9       // err: Time out occurred
#define I2C_SLAVE_ERROR 	      10      // err: Slave mode error
#define I2C_INIT_ERROR 		      11      // err: Initialization (not done)
#define I2C_RETRIES 		        12      // err: Initialization (not done)

typedef enum {
	WRITE=0,	                      //transmit
	READ=1, 	                      //receive
	WRITETHENREAD = 2
} LPC_I2C_TransMode_t;

typedef struct {
	unsigned char address;	        // slave address to sent/receive message
	unsigned char nrBytes;	        // number of bytes in message buffer
	unsigned char *buf;		          // pointer to application message buffer
	LPC_I2C_TransMode_t transMode;	// write or read flag
	unsigned char nrWriteBytes ;	  // write byte number, only used in "WriteThenRead" mode

	int dataCount ;	                // count the Tx/Rx number
} LPC_I2C_Msg_t;

// Declare the API functions
int I2C_InitMaster (unsigned long BusSpeed);
int I2C_MasterWrite (unsigned char addr, unsigned char *pMsg , unsigned long numMsg);
int I2C_MasterRead (unsigned char addr, unsigned char *pMsg , unsigned long numMsg);

int I2C_Transfer (unsigned char addr, unsigned char *pMsg , unsigned long numMsg,
	LPC_I2C_TransMode_t transMode, unsigned long numWrite);

void I2C_HandleMasterState(void);

#endif // __I2C0_DRV_H
