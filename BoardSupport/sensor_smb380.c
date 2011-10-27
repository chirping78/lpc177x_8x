/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : smb380_drv.c
 *    Description : SMB380 acceleration sensor driver (I2C data mode)
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
#include "sensor_smb380.h"
#include "lpc177x_8x_i2c.h"
#include "lpc177x_8x_pinsel.h"

/*************************************************************************
 * Function Name: SMB380_ReadWrite
 * Parameters:  txdata  point to buffer of data which will be sent.
 *                     txlen     the length of transmit buffer
 *                     rxdata point to receive buffer
 *                     rxlen     the length of receive buffer
 *
 * Return: SMB380_Status_t
 *
 * Description: Read/Write data to SMB380
 *
 *************************************************************************/
static SMB380_Status_t SMB380_ReadWrite(uint8_t* txdata, uint32_t txlen, 
	                                      uint8_t* rxdata, uint32_t rxlen)
{
	I2C_M_SETUP_Type i2cData;
	
	i2cData.sl_addr7bit = SMB380_ADDR;
	i2cData.tx_length = txlen;
    i2cData.tx_data = txdata;
    i2cData.rx_data = rxdata;
	i2cData.rx_length = rxlen;
	i2cData.retransmissions_max = 3;	
	
	if (I2C_MasterTransferData(I2C_1, &i2cData, I2C_TRANSFER_POLLING) == SUCCESS)
	{		
		return SMB380_PASS;
	}

	return SMB380_ERR;
}


/*************************************************************************
 * Function Name: SMB380_Init
 * Parameters: none
 *
 * Return: SMB380_Status_t
 *
 * Description: SMB380 init
 *
 *************************************************************************/
SMB380_Status_t SMB380_Init(void)
{
  unsigned char Data[2];

  //Init I2C module as master
  PINSEL_ConfigPin (2, 14, 2);
  PINSEL_ConfigPin (2, 15, 2);
  I2C_Init(I2C_1, SMB380_SPEED);
  I2C_Cmd(I2C_1,ENABLE);

  Data[0] = 0x14;
  SMB380_ReadWrite(&Data[0], 1, NULL, 0);
  SMB380_ReadWrite(NULL, 0, &Data[1], 1);

  Data[1] &= ~(0x1F<<0);
  Data[1] |= (0x08<<0);
  //I2C_MasterWrite(SMB380_ADDR, &Data[0], 2);
  SMB380_ReadWrite(&Data[0], 2, NULL, 0);
  

  Data[0] = 0x15;
  SMB380_ReadWrite(&Data[0], 1, NULL, 0);
  SMB380_ReadWrite(NULL, 0, &Data[1], 1);

  Data[1] &= ~(3<<1);
  Data[1] |= ((1<<5) | (1<<0));
  SMB380_ReadWrite(&Data[0], 2, NULL, 0);

  return SMB380_PASS;
}
/*************************************************************************
 * Function Name: SMB380_GetID
 * Parameters: none
 *
 * Return: SMB380_Status_t
 *
 * Description: SMB380 get chip ID and revision
 *
 *************************************************************************/
SMB380_Status_t SMB380_GetID (uint8_t *pChipId, uint8_t *pRevision)
{
unsigned char buf[2] = {SMB380_CHIP_ID};
  //Write the address of Chip ID register
  SMB380_ReadWrite(buf, 1, NULL, 0);
  SMB380_ReadWrite(NULL, 0, buf, 1);
  *pChipId = buf[0];
  *pRevision = buf[1];

  return SMB380_PASS;
}

/*************************************************************************
 * Function Name: MB380_GetData
 * Parameters: none
 *
 * Return: SMB380_Status_t
 *
 * Description: SMB380 get data
 *
 *************************************************************************/
SMB380_Status_t SMB380_GetData (pSMB380_Data_t pData)
{
  unsigned char regaddr = SMB380_ACCX_ADDR;

  SMB380_ReadWrite(&regaddr, 1, NULL, 0);
  SMB380_ReadWrite(NULL, 0,(unsigned char *)pData, sizeof(SMB380_Data_t));

  return SMB380_PASS;
}


