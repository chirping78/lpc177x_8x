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
#include "smb380_drv.h"
#include "i2c1_drv.h"

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
  I2C_InitMaster(I2C_SPEED);

  Data[0] = 0x14;
  I2C_MasterWrite(SMB380_ADDR, &Data[0], 1);
  I2C_MasterRead(SMB380_ADDR, &Data[1], 1);

  Data[1] &= ~(0x1F<<0);
  Data[1] |= (0x08<<0);
  I2C_MasterWrite(SMB380_ADDR, &Data[0], 2);

  Data[0] = 0x15;
  I2C_MasterWrite(SMB380_ADDR, &Data[0], 1);
  I2C_MasterRead(SMB380_ADDR, &Data[1], 1);

  Data[1] &= ~(3<<1);
  Data[1] |= ((1<<5) | (1<<0));
  I2C_MasterWrite(SMB380_ADDR, &Data[0], 2);

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
  I2C_MasterWrite(SMB380_ADDR, buf, 1);

  I2C_MasterRead(SMB380_ADDR, buf, 2);
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

  I2C_MasterWrite(SMB380_ADDR, &regaddr, 1);

  I2C_MasterRead(SMB380_ADDR, (unsigned char *)pData, sizeof(SMB380_Data_t));

  return SMB380_PASS;
}


/*************************************************************************
 * Function Name: SMB380_IntClear
 * Parameters: none
 *
 * Return: SMB380_Status_t
 *
 * Description: SMB380 clear interrupt
 *
 *************************************************************************/
//SMB380_Status_t SMB380_IntClear (void)
//{
//unsigned char Data[2];
//
//  Data[0] = 0x0A;
//
//  I2C_MasterWrite(SMB380_ADDR, &Data[0], 1);
//
//  I2C_MasterRead(SMB380_ADDR, &Data[1], 1);
//
//  Data[1] |= (1<<6);
//
//  I2C_MasterWrite(SMB380_ADDR, &Data[0], 2);
//}
