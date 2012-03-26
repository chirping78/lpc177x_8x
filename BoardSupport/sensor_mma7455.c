/**********************************************************************
* $Id$		Sensor_mma7455.c			2012-03-22
*//**
* @file		Sensor_mma7455.c
* @brief		MMA7455 acceleration sensor driver (I2C data mode)
* @version	1.0
* @date		22. March. 2012
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
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/

#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc177x_8x_libcfg.h"
#else
#include "lpc177x_8x_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _I2C
#include "sensor_mma7455.h"
#include "lpc177x_8x_i2c.h"
#include "lpc177x_8x_pinsel.h"

/*********************************************************************//**
 * @brief 		Read/Write data to MMA7455
 * @param[in]	 txdata  point to buffer of data which will be sent.
 * @param[in]   txlen     the length of transmit buffer
 * @param[in]  rxdata point to receive buffer
 * @param[in]  rxlen     the length of receive buffer
 * @return 		None
 **********************************************************************/
MMA7455_Status_t MMA7455_ReadWrite(uint8_t* txdata, uint32_t txlen,
	                                      uint8_t* rxdata, uint32_t rxlen)
{
	I2C_M_SETUP_Type i2cData;
	
	i2cData.sl_addr7bit = MMA7455_ADDR;
	i2cData.tx_length = txlen;
    i2cData.tx_data = txdata;
    i2cData.rx_data = rxdata;
	i2cData.rx_length = rxlen;
	i2cData.retransmissions_max = 3;	
	
	if (I2C_MasterTransferData(I2C_0, &i2cData, I2C_TRANSFER_POLLING) == SUCCESS)
	{		
		return MMA7455_PASS;
	}

	return MMA7455_ERR;
}

/*********************************************************************//**
 * @brief 		MMA7455 init
 * @param[in]	 None
 * @return 	 MMA7455_Status_t
 **********************************************************************/
MMA7455_Status_t MMA7455_Init(void)
{
  unsigned char Data[2];
  MMA7455_Status_t ret;

  //Init I2C module as master
  PINSEL_ConfigPin (0, 27, 1);
  PINSEL_ConfigPin (0, 28, 1);
  I2C_Init(I2C_0, MMA7455_SPEED);
  I2C_Cmd(I2C_0,I2C_MASTER_MODE, ENABLE);

  Data[0] = MMA7455_MODE_ADDR;
  Data[1] = MMA7455_MODE_SENS_16|MMA7455_MODE_MEASUREMENT;
  ret = MMA7455_ReadWrite(&Data[0], 2, NULL, 0);
  if(ret != MMA7455_PASS)
    return ret;
  
  Data[0] = MMA7455_CTR1_ADDR;
  Data[1] = MMA7455_CTR1_BANDWIDTH_125;
  return MMA7455_ReadWrite(&Data[0], 2, NULL, 0);
}

/*********************************************************************//**
 * @brief 		Get User Info
 * @param[in]	 UserInfo address of the variable which is used to stored User Info.
 * @return 	 MMA7455_Status_t
 **********************************************************************/
 MMA7455_Status_t MMA7455_GetUserInfo (uint8_t *UserInfo)
{
  unsigned char buf[1] = {MMA7455_USER_INFO_ADDR};
  return MMA7455_ReadWrite(buf, 1, UserInfo, 1);
}


/*********************************************************************//**
 * @brief 		MMA7455 get data
 * @param[in]	 pData address of the variable which is used to stored data.
 * @return 	 MMA7455_Status_t
 **********************************************************************/
 MMA7455_Status_t MMA7455_GetData (pMMA7455_Data_t pData)
{
  unsigned char buf[2];

  // Get Status
  while(1)
  {
    buf[0] = MMA7455_STS_ADDR;
    MMA7455_ReadWrite(buf, 1,&buf[1], 1);
    if((buf[0] & MMA7455_STS_DRDY) == MMA7455_STS_DRDY)
        break;
  }
  buf[0] = MMA7455_OUTPUT_X_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  pData->AccX = buf[1];

   buf[0] = MMA7455_OUTPUT_Y_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  pData->AccY = buf[1];

  buf[0] = MMA7455_OUTPUT_Z_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  pData->AccZ = buf[1];

  return MMA7455_PASS;
}

#endif /*_I2C*/

