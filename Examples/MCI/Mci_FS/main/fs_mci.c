/***
 * @file		fs_mci.c
 * @purpose		Drivers for SD
 * @version		1.0
 * @date		23. February. 2012
 * @author		NXP MCU SW Application Team
 *---------------------------------------------------------------------
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

#include "LPC177x_8x.h"
#include "lpc_types.h"
#include "lpc177x_8x_mci.h"
#include "lpc177x_8x_gpdma.h"
#include "debug_frmwrk.h"
#include "bsp.h"
#include "diskio.h"
#include "fs_mci.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup MCI_FS	MCI File System
 * @ingroup MCI_Examples
 * @{
 */
 
#define DMA_SIZE        (1000UL)
#define DMA_SRC			LPC_PERI_RAM_BASE		/* This is the area original data is stored
										or data to be written to the SD/MMC card. */
#define DMA_DST			(DMA_SRC+DMA_SIZE)		/* This is the area, after writing to the SD/MMC,
										data read from the SD/MMC card. */

/* treat WriteBlock as a constant address */
volatile uint8_t *WriteBlock = (uint8_t *)(DMA_SRC);

/* treat ReadBlock as a constant address */
volatile uint8_t *ReadBlock  = (uint8_t *)(DMA_DST);

/* Disk Status */
static volatile DSTATUS Stat = STA_NOINIT;	

/* 100Hz decrement timer stopped at zero (disk_timerproc()) */
static volatile WORD Timer1, Timer2;	


/* Card Configuration */
CARDCONFIG CardConfig;      /* Card configuration */


#if MCI_DMA_ENABLED
/******************************************************************************
**  DMA Handler
******************************************************************************/
void DMA_IRQHandler (void)
{
   MCI_DMA_IRQHandler();
}
#endif

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                  */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
)
{
	
	if (drv) return STA_NOINIT;			/* Supports only single drive */
//	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	/* Reset */
	Stat = STA_NOINIT;

#if MCI_DMA_ENABLED
	/* on DMA channel 0, source is memory, destination is MCI FIFO. */
	/* On DMA channel 1, source is MCI FIFO, destination is memory. */
	GPDMA_Init();
#endif

    /* For the SD card I tested, the minimum required block length is 512 */
	/* For MMC, the restriction is loose, due to the variety of SD and MMC
	card support, ideally, the driver should read CSD register to find the
	right speed and block length for the card, and set them accordingly.
	In this driver example, it will support both MMC and SD cards, and it
	does read the information by send SEND_CSD to poll the card status,
	however, to simplify the example, it doesn't configure them accordingly
	based on the CSD register value. This is not intended to support all
	the SD and MMC cards. */

	if(MCI_Init(BRD_MCI_POWERED_ACTIVE_LEVEL) != MCI_FUNC_OK)
	{
        return Stat;
	}
	

	if(mci_read_configuration() == TRUE)
	{
		Stat &= ~STA_NOINIT;
	}
	else
	{
		Stat |=  STA_NODISK;
	}
	return Stat;		
	
}
/**
  * @brief  Read card configuration and fill structure CardConfig.
  *
  * @param  None
  * @retval TRUE or FALSE. 
  */
Bool mci_read_configuration (void)
{
	uint32_t c_size, c_size_mult, read_bl_len;
	
	do
	{
		/* Get Card Type */
		CardConfig.CardType= MCI_GetCardType();
		if(CardConfig.CardType == MCI_CARD_UNKNOWN)
		{
			break;
		}

		/* Read OCR */
		/*if(MCI_ReadOCR(&CardConfig.OCR) != MCI_FUNC_OK)
		{
			break;
		} */
	    /* Read CID */
		if (MCI_GetCID(&CardConfig.CardID) != MCI_FUNC_OK)
		{
			break;
		}

		/* Set Address */
		if(MCI_SetCardAddress() != MCI_FUNC_OK)
		{
			break;
		}
		CardConfig.CardAddress = MCI_GetCardAddress();

		/* Read CSD */
		if(MCI_GetCSD((uint32_t*)CardConfig.CSD) != MCI_FUNC_OK)
		{
			break;
		}	

		/* sector size */
    	CardConfig.SectorSize = 512;
		
		 /* sector count */
	    if ((CardConfig.CardType == MCI_SDSC_V2_CARD)|| /* CSD V2.0 (for High/eXtended Capacity) */
            (CardConfig.CardType == MCI_SDHC_SDXC_CARD))
	    {
	        /* Read C_SIZE */
	        c_size =  (((uint32_t)CardConfig.CSD[7]<<16) + ((uint32_t)CardConfig.CSD[8]<<8) + CardConfig.CSD[9]) & 0x3FFFFF;
	        /* Calculate sector count */
	       CardConfig.SectorCount = (c_size + 1) * 1024;

	    } else   /* CSD V1.0 (for Standard Capacity) */
	    {
	        /* C_SIZE */
	        c_size = (((uint32_t)(CardConfig.CSD[6]&0x3)<<10) + ((uint32_t)CardConfig.CSD[7]<<2) + (CardConfig.CSD[8]>>6)) & 0xFFF;
	        /* C_SIZE_MUTE */
	        c_size_mult = ((CardConfig.CSD[9]&0x3)<<1) + ((CardConfig.CSD[10]&0x80)>>7);
	        /* READ_BL_LEN */
	        read_bl_len = CardConfig.CSD[5] & 0xF;
	        /* sector count = BLOCKNR*BLOCK_LEN/512, we manually set SECTOR_SIZE to 512*/
	        //CardConfig.SectorCount = (c_size+1)*(1<<read_bl_len) * (1<<(c_size_mult+2)) / 512;
	        CardConfig.SectorCount = (c_size+1) << (read_bl_len + c_size_mult - 7);        
	    }

        /* Get erase block size in unit of sector */
        switch (CardConfig.CardType)
        {
            case MCI_MMC_CARD:
                //CardConfig.blocksize = ((uint16_t)((CardConfig.csd[10] & 124) >> 2) + 1) * (((CardConfig.csd[11] & 3) << 3) + ((CardConfig.csd[11] & 224) >> 5) + 1);
                CardConfig.BlockSize = ((uint16_t)((CardConfig.CSD[10] & 124) >> 2) + 1) * (((CardConfig.CSD[10] & 3) << 3) + ((CardConfig.CSD[11] & 224) >> 5) + 1);
                break;
            case MCI_SDHC_SDXC_CARD:
            case MCI_SDSC_V2_CARD:
            case MCI_SDSC_V1_CARD:
                CardConfig.BlockSize = (((CardConfig.CSD[10] & 63) << 1) + ((uint16_t)(CardConfig.CSD[11] & 128) >> 7) + 1) << ((CardConfig.CSD[13] >> 6) - 1);
                break;
            default:
                break;                
        }

		/* Select Card */
		if(MCI_Cmd_SelectCard() != MCI_FUNC_OK)
		{
			break;
		}

		if ((CardConfig.CardType== MCI_SDSC_V1_CARD) ||
			(CardConfig.CardType== MCI_SDSC_V2_CARD) ||
			(CardConfig.CardType== MCI_SDHC_SDXC_CARD)) 
		{
				MCI_Set_MCIClock( MCI_NORMAL_RATE );
		
				if (MCI_SetBusWidth( SD_4_BIT ) != MCI_FUNC_OK )
				{
					break;
				}
		}

		/* For SDHC or SDXC, block length is fixed to 512 bytes, for others,
         the block length is set to 512 manually. */
        if (CardConfig.CardType == MCI_MMC_CARD ||
            CardConfig.CardType == MCI_SDSC_V1_CARD ||
            CardConfig.CardType == MCI_SDSC_V2_CARD )
        {
    		if(MCI_SetBlockLen(BLOCK_LENGTH) != MCI_FUNC_OK)
    		{
    			break;
    		}
        }
        
        return TRUE;
	}
	while (FALSE);

    return FALSE;
}

Bool mci_wait_for_ready (void)
{
	int32_t  cardSts;
    Timer2 = 50;    // 500ms
    while(Timer2)
	{	
		if(MCI_GetCardStatus(&cardSts) == MCI_FUNC_OK)
		{
			return TRUE;
		}
			
	}
    return FALSE;    
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, *ptr = buff;

	if (drv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;

	switch (ctrl) {
	case CTRL_SYNC :		/* Make sure that no pending write process */
		if(mci_wait_for_ready() == TRUE)
		{
			res = RES_OK;
		}
		
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
		*(DWORD*)buff = CardConfig.SectorCount;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
		*(WORD*)buff = CardConfig.SectorSize;	//512;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		*(DWORD*)buff = CardConfig.BlockSize;
		res = RES_OK;
		break;

	case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
		*ptr = CardConfig.CardType;
		res = RES_OK;
		break;

	case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
		for (n=0;n<16;n++)
			*(ptr+n) = CardConfig.CSD[n]; 
		res = RES_OK;
		break;

	case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
        {
            uint8_t* cid = (uint8_t*) &CardConfig.CardID;
    		for (n=0;n<16;n++)
    			*(ptr+n) = cid[n];
        }
		res = RES_OK;
		break;

	case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
        {
            uint8_t* ocr = (uint8_t*)&CardConfig.OCR;
    		for (n=0;n<4;n++)
    			*(ptr+n) = ocr[n];
        }
		res = RES_OK;
		break;

	case MMC_GET_SDSTAT :	/* Receive SD status as a data block (64 bytes) */
		{
			int32_t cardStatus;
			if(MCI_GetCardStatus(&cardStatus) == MCI_FUNC_OK) 
			{
                uint8_t* status = (uint8_t*)&cardStatus;
				for (n=0;n<2;n++)
           			 *(ptr+n) = ((uint8_t*)status)[n];
				res = RES_OK;   
			}
		}
		break;

	default:
		res = RES_PARERR;
	}

	return res;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE drv,			/* Physical drive number (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (MCI_ReadBlock (buff, sector, count) == MCI_FUNC_OK)	
	{
		//while(MCI_GetBlockXferEndState() != 0);
		while(MCI_GetDataXferEndState() != 0);

		if(count > 1)
		{
			if(MCI_Cmd_StopTransmission()  != MCI_FUNC_OK)
				return RES_ERROR;
		}
		return RES_OK;
	}
	else
		return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */

	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
    uint32_t tmp;

	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
//	if (Stat & STA_PROTECT) return RES_WRPRT;
    for(tmp = 0x100000;tmp;tmp--);

	if ( MCI_WriteBlock((uint8_t*)buff, sector, count) == MCI_FUNC_OK)
	{
		//while(MCI_GetBlockXferEndState() != 0);
		while(MCI_GetDataXferEndState() != 0);

		if(count > 1)
		{
			if(MCI_Cmd_StopTransmission()  != MCI_FUNC_OK)
				return RES_ERROR;
		}
		return RES_OK;
	}
	else
		return 	RES_ERROR;

}
#endif /* _READONLY == 0 */


/*-----------------------------------------------------------------------*/
/* Device timer function  (Platform dependent)                           */
/*-----------------------------------------------------------------------*/
/* This function must be called from timer interrupt routine in period
/  of 10 ms to generate card control timing.
*/
void disk_timerproc (void)
{
    WORD n;

	n = Timer1;						/* 100Hz decrement timer stopped at 0 */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;               
}

/******************************************************************************
**                            End Of File
******************************************************************************/

/**
 * @}
 */
 
