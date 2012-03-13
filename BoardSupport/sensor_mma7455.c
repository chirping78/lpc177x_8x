/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : MMA7455_drv.c
 *    Description : MMA7455 acceleration sensor driver (I2C data mode)
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
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc177x_8x_libcfg.h"
#else
#include "lpc177x_8x_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _I2C
#include "sensor_mma7455.h"
#include "lpc177x_8x_i2c.h"
#include "lpc177x_8x_pinsel.h"

/*************************************************************************
 * Function Name: MMA7455_ReadWrite
 * Parameters:  txdata  point to buffer of data which will be sent.
 *                     txlen     the length of transmit buffer
 *                     rxdata point to receive buffer
 *                     rxlen     the length of receive buffer
 *
 * Return: MMA7455_Status_t
 *
 * Description: Read/Write data to MMA7455
 *
 *************************************************************************/
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


/*************************************************************************
 * Function Name: MMA7455_Init
 * Parameters: none
 *
 * Return: MMA7455_Status_t
 *
 * Description: MMA7455 init
 *
 *************************************************************************/
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
  Data[1] = MMA7455_CTR1_XDA_DISABLE|MMA7455_CTR1_YDA_DISABLE|MMA7455_CTR1_ZDA_DISABLE;
  return MMA7455_ReadWrite(&Data[0], 2, NULL, 0);
}
/*************************************************************************
 * Function Name: MMA7455_GetUserInfo
 * Parameters: UserInfo address of the variable which is used to stored User Info.
 *
 * Return: MMA7455_Status_t
 *
 * Description: Get User Info
 *
 *************************************************************************/
MMA7455_Status_t MMA7455_GetUserInfo (uint8_t *UserInfo)
{
  unsigned char buf[1] = {MMA7455_USER_INFO_ADDR};
  return MMA7455_ReadWrite(buf, 1, UserInfo, 1);
}

/*************************************************************************
 * Function Name: MB380_GetData
 * Parameters: none
 *
 * Return: MMA7455_Status_t
 *
 * Description: MMA7455 get data
 *
 *************************************************************************/
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

