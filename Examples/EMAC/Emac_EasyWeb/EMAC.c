/******************************************************************
 *****                                                        *****
 *****  Name: cs8900.c                                        *****
 *****  Ver.: 1.0                                             *****
 *****  Date: 07/05/2001                                      *****
 *****  Auth: Andreas Dannenberg                              *****
 *****        HTWK Leipzig                                    *****
 *****        university of applied sciences                  *****
 *****        Germany                                         *****
 *****  Func: ethernet packet-driver for use with LAN-        *****
 *****        controller CS8900 from Crystal/Cirrus Logic     *****
 *****                                                        *****
 *****  Keil: Module modified for use with Philips            *****
 *****        LPC2478 EMAC Ethernet controller                *****
 *****                                                        *****
 ******************************************************************/

#include "EMAC.h"
#include "tcpip.h"
#include "LPC177x_8x.h"
#include "lpc177x_8x_emac.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_clkpwr.h"

uint16_t *rptr;
static uint16_t *tptr;

// configure port-pins for use with LAN-controller,
// reset it and send the configuration-sequence
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void Init_EMAC(void)
{
// Initializes the LPC_EMAC ethernet controller
	unsigned int delay;
	/* LPC_EMAC configuration type */
	EMAC_CFG_Type Emac_Config;

	/* LPC_EMAC address */
	uint8_t EMACAddr[] = {MYMAC_1, MYMAC_2, MYMAC_3, MYMAC_4, MYMAC_5, MYMAC_6};

	/* Enable P1 Ethernet Pins. */
	/* on rev. 'A' and later, P1.6 should NOT be set. */
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

	Emac_Config.Mode = EMAC_MODE_AUTO;
	Emac_Config.pbEMAC_Addr = EMACAddr;
	// Initialize LPC_EMAC module with given parameter
	while (EMAC_Init(&Emac_Config) == ERROR)
	{
	// Delay for a while then continue initializing LPC_EMAC module
		for (delay = 0x100000; delay; delay--);
	}

}


// reads a word in little-endian byte order from RX_BUFFER
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
uint16_t ReadFrame_EMAC(void)
{
	return (*rptr++);
}

// reads a word in big-endian byte order from RX_FRAME_PORT
// (useful to avoid permanent byte-swapping while reading
// TCP/IP-data)
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
uint16_t ReadFrameBE_EMAC(void)
{
  uint16_t ReturnValue;

  ReturnValue = SwapBytes (*rptr++);
  return (ReturnValue);
}


// copies bytes from frame port to MCU-memory
// NOTES: * an odd number of byte may only be transfered
//          if the frame is read to the end!
//        * MCU-memory MUST start at word-boundary
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void CopyFromFrame_EMAC(void *Dest, uint16_t Size)
{
	uint16_t * piDest;
	piDest = Dest;
	while (Size > 1)
	{
		*piDest++ = ReadFrame_EMAC();
		Size -= 2;
	}

	if (Size)
	{
	/* check for leftover byte...
	the LAN-Controller will return 0
	for the highbyte*/
		*(uint8_t *)piDest = (char)ReadFrame_EMAC();
	}
}

// does a dummy read on frame-I/O-port
// NOTE: only an even number of bytes is read!
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void DummyReadFrame_EMAC(uint16_t Size)
{
/* discards an EVEN number of bytes from RX-fifo */

	while (Size > 1)
	{
		ReadFrame_EMAC();
		Size -= 2;
	}
}

// Reads the length of the received ethernet frame and checks if the
// destination address is a broadcast message or not
// returns the frame length
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
uint16_t StartReadFrame(void)
{
	uint16_t RxLen;
	RxLen = EMAC_GetReceiveDataSize();
	rptr = (uint16_t *)EMAC_GetReadPacketBuffer();
	return(RxLen);
}

/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void EndReadFrame(void)
{
 	EMAC_UpdateRxConsumeIndex();
}

/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
unsigned int CheckFrameReceived(void)
{
	if (EMAC_CheckReceiveIndex() == TRUE)
	{
		return (1);
	}
	else
	{
	return (0);
	}
}

// requests space in LPC_EMAC memory for storing an outgoing frame
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void RequestSend(uint16_t FrameSize)
{
	tptr = (uint16_t *)EMAC_RequestSend(FrameSize);
}

// check if ethernet controller is ready to accept the
// frame we want to send
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
unsigned int Rdy4Tx(void)
{
/* the ethernet controller transmits much faster than the CPU can load its buffers */
	return (1);
}


// writes a word in little-endian byte order to TX_BUFFER
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void WriteFrame_EMAC(uint16_t Data)
{
	*tptr++ = Data;
}

// copies bytes from MCU-memory to frame port
// NOTES: * an odd number of byte may only be transfered
//          if the frame is written to the end!
//        * MCU-memory MUST start at word-boundary
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void CopyToFrame_EMAC(void *Source, unsigned int Size)
{
	uint16_t * piSource;

	piSource = Source;
	Size = (Size + 1) & 0xFFFE;    // round Size up to next even number
	while (Size > 0)
	{
		WriteFrame_EMAC(*piSource++);
		Size -= 2;
	}
	EMAC_UpdateTxProduceIndex();
}

