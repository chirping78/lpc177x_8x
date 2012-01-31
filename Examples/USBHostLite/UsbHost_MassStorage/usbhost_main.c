/**********************************************************************
* $Id$		usbhost_main.c			2011-09-05
*//**
* @file		usbhost_main.c
* @brief	Demo for USB Host Controller.
* @version	1.0
* @date		05. September. 2011
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

/*
**************************************************************************************************************
*                                       INCLUDE HEADER FILES
**************************************************************************************************************
*/
#include "debug_frmwrk.h"
#include  "usbhost_main.h"

/** @defgroup USBHost_MassStorage	USB Host Controller for Mass Storage Device
 * @ingroup USBHostLite_Examples 
 * @{
 */

/** @defgroup USBHost_Fat	 Fat File System
 * @ingroup USBHost_MassStorage
 * @{
 */

/**
 * @}
 */


/** @defgroup USBHost_Ms  USB Host Mass Storage Class
 * @ingroup USBHost_MassStorage
 * @{
 */

/**
 * @}
 */

/** @defgroup USBHost_Uart  USB Host Debug
 * @ingroup USBHost_MassStorage
 * @{
 */

/**
 * @}
 */

uint8_t menu[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART Host Lite example \n\r"
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
"\t - UART Communication: 115200 bps \n\r"
" This example used to test USB Host function.\n\r"
"********************************************************************************\n\r";
/*********************************************************************//**
 * @brief		Print menu
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void print_menu(void)
{
	_DBG_(menu);
}
/*********************************************************************//**
 * @brief 			This function is the main function where the execution begins
 * @param[in]		None
 * @return 		    None
 **********************************************************************/
int main()
{
    int32_t  rc;
    uint32_t  numBlks, blkSize;					\
    uint8_t  inquiryResult[INQUIRY_LENGTH];

    //SystemInit();

    debug_frmwrk_init();
		
	print_menu();
		
    Host_Init();               /* Initialize the lpc17xx host controller                                    */

    PRINT_Log("Host Initialized\n");
    PRINT_Log("Connect a Mass Storage device\n");
	
    rc = Host_EnumDev();       /* Enumerate the device connected                                            */
    if ((rc == USB_HOST_FUNC_OK) && 
	(Host_GetDeviceType() == MASS_STORAGE_DEVICE)) {
		
	PRINT_Log("Mass Storage device connected\n");

	/* Initialize the mass storage and scsi interfaces */
        rc = MS_Init( &blkSize, &numBlks, inquiryResult );
        if (rc == MS_FUNC_OK) {
            rc = FAT_Init();   /* Initialize the FAT16 file system                                          */
            if (rc == FAT_FUNC_OK) {
                Main_Copy();   /* Call the application                                                      */
            } else {
                return (0);
            }
        } else {
            return (0);
        }
    } else {
	    PRINT_Log("Not a Mass Storage device\n");							
        return (0);
    }
    while(1);
}

/*********************************************************************//**
 * @brief 			This function is used by the user to read data from the disk 
 * @param[in]		None
 * @return 		    None
 **********************************************************************/
void  Main_Read (void)
{
    int32_t  fdr;
    uint32_t  bytes_read;
    

    fdr = FILE_Open(FILENAME_R, RDONLY);
    if (fdr > 0) {
        PRINT_Log("Reading from %s...\n", FILENAME_R);
        do {
            bytes_read = FILE_Read(fdr, UserBuffer, MAX_BUFFER_SIZE);
        } while (bytes_read);

        FILE_Close(fdr);
        PRINT_Log("Read Complete\n");
    } else {
        PRINT_Log("Could not open file %s\n", FILENAME_R);
        return;
    }
}

/*********************************************************************//**
 * @brief 			This function is used by the user to write data to disk 
 * @param[in]		None
 * @return 		    None
 **********************************************************************/

void  Main_Write (void)
{
    int32_t  fdw;
    int32_t  fdr;
    uint32_t  tot_bytes_written;
    uint32_t  bytes_written;


    fdr = FILE_Open(FILENAME_R, RDONLY);
    if (fdr > 0) {
        FILE_Read(fdr, UserBuffer, MAX_BUFFER_SIZE);
        fdw = FILE_Open(FILENAME_W, RDWR);
        if (fdw > 0) {
            tot_bytes_written = 0;
            PRINT_Log("Writing to %s...\n", FILENAME_W);
            do {
                bytes_written = FILE_Write(fdw, UserBuffer, MAX_BUFFER_SIZE);
                tot_bytes_written += bytes_written;
            } while (tot_bytes_written < WRITE_SIZE);
            FILE_Close(fdw);
            PRINT_Log("Write completed\n");
        } else {
            PRINT_Log("Could not open file %s\n", FILENAME_W);
            return;
        }
        FILE_Close(fdr);
    } else {
        PRINT_Log("Could not open file %s\n", FILENAME_R);
        return;
    }
}

/*********************************************************************//**
 * @brief 			This function is used by the user to copy a file 
 * @param[in]		None
 * @return 		    None
 **********************************************************************/

void  Main_Copy (void)
{
    int32_t  fdr;
    int32_t  fdw;
    uint32_t  bytes_read;


    fdr = FILE_Open(FILENAME_R, RDONLY);
    if (fdr > 0) {
        fdw = FILE_Open(FILENAME_W, RDWR);
        if (fdw > 0) {
            PRINT_Log("Copying file...\n");
            do {
                bytes_read = FILE_Read(fdr, UserBuffer, MAX_BUFFER_SIZE);
                FILE_Write(fdw, UserBuffer, bytes_read);
            } while (bytes_read);
            FILE_Close(fdw);
        } else {
            PRINT_Log("Could not open file %s\n", FILENAME_W);
            return;
        }
        FILE_Close(fdr);
        PRINT_Log("Copy completed\n");
    } else {
        PRINT_Log("Could not open file %s\n", FILENAME_R);
        return;
    }
}
