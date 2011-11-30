#include "emac.h"
#include "lpc177x_8x_emac.h"
#include "lpc177x_8x_pinsel.h"
#include "phylan.h"
#include <string.h>
#include <stdio.h>


/* For debugging... */
#include "debug_frmwrk.h"
#include <stdio.h>

/* Example group ----------------------------------------------------------- */
/** @defgroup EMAC_uIP	uIP
 * @ingroup EMAC_Examples
 * @{
 */

#define DB	_DBG((uint8_t *)db)
char db[64];
static uint16_t *rptr = NULL;
static uint32_t rx_size[EMAC_MAX_FRAME_NUM];
static uint16_t* rx_ptr[EMAC_MAX_FRAME_NUM];
static uint8_t rx_done = 0, read_done = 0;


/* Init the LPC17xx ethernet */
BOOL_8 tapdev_init(void)
{
	/* EMAC configuration type */
	EMAC_CFG_Type Emac_Config;
	/* EMAC address */
	uint8_t EMACAddr[] = {EMAC_ADDR0, EMAC_ADDR1, EMAC_ADDR2, \
						EMAC_ADDR3, EMAC_ADDR4, EMAC_ADDR5};

#if AUTO_NEGOTIATION_ENA != 0
	Emac_Config.PhyCfg.Mode = EMAC_MODE_AUTO;
#else
	#if (FIX_SPEED == SPEED_100)
		#if (FIX_DUPLEX == FULL_DUPLEX)
			Emac_Config.Mode = EMAC_MODE_100M_FULL;
		#elif (FIX_DUPLEX == HALF_DUPLEX)
			Emac_Config.Mode = EMAC_MODE_100M_HALF;
		#else
			#error Does not support this duplex option
		#endif
	#elif (FIX_SPEED == SPEED_10)
		#if (FIX_DUPLEX == FULL_DUPLEX)
				Emac_Config.Mode = EMAC_MODE_10M_FULL;
		#elif (FIX_DUPLEX == HALF_DUPLEX)
				Emac_Config.Mode = EMAC_MODE_10M_HALF;
		#else
			#error Does not support this duplex option
		#endif
	#else
		#error Does not support this speed option
	#endif
#endif

	/*
	 * Enable P1 Ethernet Pins:
	 * P1.0 - ENET_TXD0
	 * P1.1 - ENET_TXD1
	 * P1.4 - ENET_TX_EN
	 * P1.8 - ENET_CRS
	 * P1.9 - ENET_RXD0
	 * P1.10 - ENET_RXD1
	 * P1.14 - ENET_RX_ER
	 * P1.15 - ENET_REF_CLK
	 * P1.16 - ENET_MDC
	 * P1.17 - ENET_MDIO
	 */
	PINSEL_ConfigPin(1,0,1);
	PINSEL_ConfigPin(1,1,1);
	PINSEL_ConfigPin(1,4,1);
	PINSEL_ConfigPin(1,8,1);
	PINSEL_ConfigPin(1,9,1);
	PINSEL_ConfigPin(1,10,1);
	PINSEL_ConfigPin(1,14,1);
	PINSEL_ConfigPin(1,15,1);
	PINSEL_ConfigPin(1,16,1);
	PINSEL_ConfigPin(1,17,1);

	_DBG_("Init EMAC module");
	sprintf(db,"MAC addr: %X-%X-%X-%X-%X-%X \n\r", \
			 EMACAddr[0],  EMACAddr[1],  EMACAddr[2], \
			  EMACAddr[3],  EMACAddr[4],  EMACAddr[5]);
	DB;

	Emac_Config.PhyCfg.Mode = EMAC_MODE_AUTO;
	Emac_Config.pbEMAC_Addr = EMACAddr;
	Emac_Config.bPhyAddr = EMAC_PHY_DEFAULT_ADDR;
	Emac_Config.nMaxFrameSize = 1536;
	Emac_Config.pfnPHYInit = PHY_Init;
	Emac_Config.pfnPHYReset = PHY_Reset;
	Emac_Config.pfnFrameReceive = tapdev_frame_receive_cb;
	Emac_Config.pfnErrorReceive = tapdev_error_receive_cb;
	Emac_Config.pfnTransmitFinish = NULL;
	Emac_Config.pfnSoftInt = NULL;
	Emac_Config.pfnWakeup = NULL;

	// Initialize EMAC module with given parameter
	if (EMAC_Init(&Emac_Config) == ERROR){
		return (FALSE);
	}

	_DBG_("Init EMAC complete");

	return (TRUE);
}

/* save the pointer to received frame buffer*/
void tapdev_frame_receive_cb(uint16_t* pData, uint32_t size)
{

  rx_ptr[rx_done] = pData;
  rx_size[rx_done] = size;
  rx_done++;
  if(rx_done >= EMAC_MAX_FRAME_NUM)
     rx_done = 0;
}

/* Handle errors */
void tapdev_error_receive_cb(int32_t errCode)
{
  
}

/* read a half-word value from frame buffer */
uint16_t tapdev_read_half_word(void)
{
	return (*rptr++);
}

/* swap a half-word value */
uint16_t tapdev_swap_bytes(uint16_t Data)
{
  return (Data >> 8) | (Data << 8);
}

/* read a half-word value in big-endian order from frame buffer */
uint16_t tapdev_read_half_word_be(void)
{
  uint16_t ReturnValue;

  ReturnValue = tapdev_swap_bytes (*rptr++);
  return (ReturnValue);
}

/* get the pointer to the received frame buffer */
uint16_t tapdev_start_read_frame(void)
{
	rptr = rx_ptr[read_done];
	read_done++;
	if(read_done >= EMAC_MAX_FRAME_NUM)
	   read_done = 0;
	return(rx_size[read_done]);
}

/* notify that frame reading ends */
void tapdev_end_read_frame(void)
{
 	rx_size[read_done] = 0;
}


/* receive an Ethernet frame from MAC/DMA controller */
UNS_32 tapdev_read(void * pPacket)
{
	UNS_32 Size = EMAC_MAX_PACKET_SIZE;
	UNS_32 Ret;
	
	uint16_t * piDest;

	Size = 	 rx_size[read_done];
	if(Size == 0)
	   return 0;

	Size =  MIN(Size, EMAC_MAX_PACKET_SIZE);
	Ret = Size;

	tapdev_start_read_frame();
	piDest = pPacket;
	while (Size > 1)
	{
		*piDest++ = tapdev_read_half_word();
		Size -= 2;
	}

	if (Size)
	{
	/* check for leftover byte...
	the LAN-Controller will return 0
	for the highbyte*/
		*(uint8_t *)piDest = (char)tapdev_read_half_word();
	}

	tapdev_end_read_frame();

	return 	Ret;
}

/* transmit an Ethernet frame to MAC/DMA controller */
BOOL_8 tapdev_send(void *pPacket, UNS_32 size)
{
	EMAC_PACKETBUF_Type TxPack;

	// Check size
	if(size == 0){
		return(TRUE);
	}

	// check Tx Slot is available
	if (EMAC_GetBufferSts(EMAC_TX_BUFF) != EMAC_BUFF_EMPTY){
		return (FALSE);
	}

	size = MIN(size,EMAC_MAX_PACKET_SIZE);

    TxPack.pbDataBuf= pPacket;
	TxPack.ulDataLen = size;
	EMAC_WritePacketBuffer(&TxPack);

	return(TRUE);
}

/*
 * @}
 */
