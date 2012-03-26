/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : MMA7455_drv.c
 *    Description : MMA7455 acceleration sensor driver include file
 *
 *    History :
 *    1. Date        : 13, February 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *
 *    $Revision: 24636 $
 *
 *    @Modify: NXP MCU Application Team - NguyenCao
 *    @Date: 04. March. 2011
 **************************************************************************/

#ifndef __MMA7455_DRV_H
#define __MMA7455_DRV_H

#include "lpc_types.h"

/** @defgroup  Sensor_MMA7455 	I2C Sensor MMA7455 
 * @ingroup LPC177x_8xCMSIS_Board_Support
 * @{
 */

#define MMA7455_SPEED  200000
#define MMA7455_ADDR   0x1D
#define MMA7455_I2C    (I2C_1)


#define MMA7455_OUTPUT_X_LSB_ADDR  0x00
#define MMA7455_OUTPUT_X_MSB_ADDR  0x01
#define MMA7455_OUTPUT_Y_LSB_ADDR  0x02
#define MMA7455_OUTPUT_Y_MSB_ADDR  0x03
#define MMA7455_OUTPUT_Z_LSB_ADDR  0x04
#define MMA7455_OUTPUT_Z_MSB_ADDR  0x05

#define MMA7455_OUTPUT_X_ADDR  0x06
#define MMA7455_OUTPUT_Y_ADDR  0x07
#define MMA7455_OUTPUT_Z_ADDR  0x08

#define MMA7455_STS_ADDR   0x09
#define MMA7455_STS_DRDY   0x01      // Data is ready
#define MMA7455_STS_DOVR   0x02      // Data is over written
#define MMA7455_STS_PERR   0x04      // Parity error

#define MMA7455_DETECT_SOURCE_ADDR   0x0A
#define MMA7455_DETECT_SOURCE_LEVEL_X   (0x01<<7)
#define MMA7455_DETECT_SOURCE_LEVEL_Y   (0x01<<6)
#define MMA7455_DETECT_SOURCE_LEVEL_Z   (0x01<<5)
#define MMA7455_DETECT_SOURCE_PULSE_X   (0x01<<4)
#define MMA7455_DETECT_SOURCE_PULSE_Y   (0x01<<3)
#define MMA7455_DETECT_SOURCE_PULSE_Z   (0x01<<2)

#define MMA7455_I2C_ADDR   0x0D
#define MMA7455_USER_INFO_ADDR 0x0E
#define MMA7455_WHOAMI_ADDR   0x0F

#define MMA7455_OFS_X_LSB_ADDR  0x10
#define MMA7455_OFS_X_MSB_ADDR  0x11
#define MMA7455_OFS_Y_LSB_ADDR  0x12
#define MMA7455_OFS_Y_MSB_ADDR  0x13
#define MMA7455_OFS_Z_LSB_ADDR  0x14
#define MMA7455_OFS_Z_MSB_ADDR  0x15

#define MMA7455_MODE_ADDR   0x16
#define MMA7455_MODE_STANDBY         (0x00)
#define MMA7455_MODE_MEASUREMENT     (0x01)
#define MMA7455_MODE_LEVEL_DETECT    (0x02)
#define MMA7455_MODE_PULSE_DETECT    (0x03)
#define MMA7455_MODE_SENS_16    (0x00<<2)    //8g
#define MMA7455_MODE_SENS_64    (0x01<<2)    //2g
#define MMA7455_MODE_SENS_32    (0x10<<2)    //4g
#define MMA7455_MODE_SELF_TEST   (0x01<<4)
#define MMA7455_MODE_DRPD       (0x01<<6)

#define MMA7455_CTR1_ADDR      0x18
#define MMA7455_CTR1_XDA_DISABLE    (0x01<<3)
#define MMA7455_CTR1_YDA_DISABLE    (0x01<<4)
#define MMA7455_CTR1_ZDA_DISABLE    (0x01<<5)
#define MMA7455_CTR1_BANDWIDTH_125  (0x01<<7) // Default 62.5Hz

#define MMA7455_CTRL2_ADDR     0x19



typedef int8_t MMA7455_Status_t;
#define MMA7455_PASS        0
#define MMA7455_ERR         (-1)

#pragma pack(1)
typedef struct _MMA7455_Data_t
{
  int8_t AccX;
  int8_t AccY;
  int8_t AccZ;
} MMA7455_Data_t, *pMMA7455_Data_t;

#pragma pack()

typedef enum _MMA7455_Range_t
{
  MMA7455_2G = 0, MMA7455_4G, MMA7455_8G
} MMA7455_Range_t;

typedef enum _MMA7455_Bandwidth_t
{
  MMA7455_25HZ = 0, MMA7455_50HZ, MMA7455_100HZ, MMA7455_190HZ,
  MMA7455_375HZ, MMA7455_750HZ, MMA7455_1500HZ
} MMA7455_Bandwidth_t;

/*************************************************************************
 * Function Name: MMA7455_Init
 * Parameters: none
 *
 * Return: MMA7455_Status_t
 *
 * Description: MMA7455 init
 *
 *************************************************************************/
MMA7455_Status_t MMA7455_Init(void);

MMA7455_Status_t MMA7455_ReadWrite(uint8_t* in_data, uint32_t txlen, 
	                                      uint8_t* out_data, uint32_t rxlen);
	                                      

/*************************************************************************
 * Function Name: MMA7455_GetID
 * Parameters: none
 *
 * Return: MMA7455_Status_t
 *
 * Description: MMA7455 get chip ID and revision
 *
 *************************************************************************/
MMA7455_Status_t MMA7455_GetID (uint8_t *pChipId, uint8_t *pRevision);

/*************************************************************************
 * Function Name: MMA7455_GetData
 * Parameters: none
 *
 * Return: MMA7455_Status_t
 *
 * Description:
 *
 *************************************************************************/
MMA7455_Status_t MMA7455_GetData (pMMA7455_Data_t pData);
//MMA7455_Status_t MMA7455_IntClear (void);
/**
 * @}
 */

#endif // __MMA7455_DRV_H
