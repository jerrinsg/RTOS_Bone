/*
    FreeRTOS V7.0.1 - Copyright (C) 2011 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS books - available as PDF or paperback  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

/*
	BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER FOR UART3. 

	This file contains all the serial port components that can be compiled to
	either ARM or THUMB mode.  Components that must be compiled to ARM mode are
	contained in serialISR.c.
*/

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Demo application includes. */
#include "serial.h"

/*-----------------------------------------------------------*/

/* Contants to setup Queues */

#define serINVALID_QUEUE				( ( xQueueHandle ) 0 )
#define serHANDLE						( ( xComPortHandle ) 1 )
#define serNO_BLOCK						( ( portTickType ) 0 )

/*-----------------------------------------------------------*/

/* Queues used to hold received characters, and characters waiting to be
transmitted. */
static xQueueHandle xRxedChars; 
static xQueueHandle xCharsForTx; 

/*-----------------------------------------------------------*/

/* Communication flag between the interrupt service routine and serial API. */
static volatile long *plTHREEmpty;

/* 
 * The queues are created in serialISR.c as they are used from the ISR.
 * Obtain references to the queues and THRE Empty flag. 
 */
extern void vSerialISRCreateQueues(	unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRxedChars, xQueueHandle *pxCharsForTx, long volatile **pplTHREEmptyFlag );

/*-----------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal( unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength )
{
	xComPortHandle xReturn = serHANDLE;
	unsigned int efr, mcr, lcr;
	short dll,dlh;
	extern void ( vUART_ISR_Wrapper )( void );
	
	/* The queues are used in the serial ISR routine, so are created from
	serialISR.c (which is always compiled to ARM mode. */
	vSerialISRCreateQueues( uxQueueLength, &xRxedChars, &xCharsForTx, &plTHREEmpty );

	portENTER_CRITICAL();
	{
		/* -------- BLOCK 1: Fifo Management ------------ */

		/* Software Reset 17.5.1.1.1 (2681) */
		/* Reset the UART Interface (2681)*/
		*(REG32(SERIAL_BASE + SYSC_REG)) = 0x2;
		
		/* Poll the SYSS_REG bit 0 until it equals to 1 */
		while(((*(REG32(SERIAL_BASE + SYSS_REG))) & 0x1) != 1){;}
		
		/* FIFOS and DMA Settings 17.5.1.1.2 */
		/* Swich to mode B */
		lcr=*(REG32(SERIAL_BASE + LCR_REG));
		*(REG32(SERIAL_BASE + LCR_REG)) = 0xBF;

		/* Save EFR_REG register */
		efr = *(REG32(SERIAL_BASE + EFR_REG));
		/* Enable enhanced features */
		*(REG32(SERIAL_BASE + EFR_REG)) |= 0x10;

		/* Switch to mode A */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0x80;
		
		/* Set loopback mode */
		*(REG32(SERIAL_BASE + MCR_REG)) |= 0x13;

		*(REG32(SERIAL_BASE + WER_REG)) |= 0x1;
		
		/* Save MCR Register */
		mcr = *(REG32(SERIAL_BASE + MCR_REG));
		*(REG32(SERIAL_BASE + MCR_REG)) |= 0x40;
			
		/* Enable Fifo, set trigger levels p2696. Trigger levels are ignored for now.*/
		*(REG32(SERIAL_BASE + FCR_REG)) = 0x7;
		while( (*(REG32(SERIAL_BASE + FCR_REG)) & 0x6) != 0) {;}
		*(REG32(SERIAL_BASE + FCR_REG)) = 0x0;

		/* Switch to mode B */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0xBF;

		/* Setup trigger levels for interrupt generation to 0. p2710 */
		*(REG32(SERIAL_BASE + TLR_REG)) = 0x0;
		*(REG32(SERIAL_BASE + SCR_REG)) |= 0x3F;

		/* Restore efr value */
		*(REG32(SERIAL_BASE + EFR_REG)) = efr;

		/* Switch to mode A */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0x80;

		/* Restore MCR register */
		*(REG32(SERIAL_BASE + MCR_REG)) = mcr;

		/* Restore LCR */
		*(REG32(SERIAL_BASE + LCR_REG)) = lcr;
		
		*(REG32(SERIAL_BASE + RHR_REG )) = 0;
		*(REG32(SERIAL_BASE + THR_REG )) = 0;
		*(REG32(SERIAL_BASE + LSR_REG ));
		*(REG32(SERIAL_BASE + MSR_REG ));
		*(REG32(SERIAL_BASE + IIR_REG ));

		/*----------- 17.5.1.1.3 (page 2682 )--------- */

		/* Disable UART access */
		*(REG32(SERIAL_BASE + MDR1_REG)) |= 0x7;

		/* Switch to mode B */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0xBF;

		/* Enable access to IER_REG */
		efr = *(REG32(SERIAL_BASE + EFR_REG));
		*(REG32(SERIAL_BASE + EFR_REG)) |= 0x10;

		/* Switch to register operational mode */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0x0;

		/* Set IER register to 0x0000 */
		*(REG32(SERIAL_BASE + IER_REG)) = 0x0;

		/* Switch to mode B */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0xBF;

		/* Setup divisor. Baudrate is an argument so
		 * setup registers based on user selection
		 * page: 2666
		 */
		switch(ulWantedBaud)
		{
			case 300: //300 bps
				dll = 0x27;
				dlh = 0x10;
				break;
			case 600:
				dll = 0x13;
				dlh = 0x88;
				break;
			case 1200:
				dll = 0x09;
				dlh = 0xC4;
				break;
			case 2400:
				dll = 0x04;
				dlh = 0xE2;
				break;
			case 4800:
				dll = 0x02;
				dlh = 0x71;
				break;
			case 9600:
				dll = 0x01;
				dlh = 0x38;
				break;
			case 14400:
				dll = 0x00;
				dlh = 0xD0;
				break;
			case 19200:
				dll = 0x00;
				dlh = 0x9C;
				break;
			case 28800:
				dll = 0x00;
				dlh = 0x68;
				break;
			case 38400:
				dll = 0x00;
				dlh = 0x4E;
				break;
			case 57600:
				dll = 0x00;
				dlh = 0x34;
				break;
			case 115200:
				dll = 0x00;
				dlh = 0x1A;
				break;
			case 230400:
				dll = 0x00;
				dlh = 0x0D;
				break;
			case 460800:
				dll = 0x00;
				dlh = 0x08;
				break;
			case 921600:
				dll = 0x00;
				dlh = 0x04;
				break;
			case 1843000:
				dll = 0x00;
				dlh = 0x02;
				break;
			case 3688400:
				dll = 0x00;
				dlh = 0x01;
				break;
			default:
				// falling back to 115200
				dll = 0x00;
				dlh = 0x1A;
				break;		
		}
		*(REG32(SERIAL_BASE + DLL_REG)) = dll;
		*(REG32(SERIAL_BASE + DLH_REG)) = dlh;

		/* Switch to operational mode */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0x0;

		/* Load Interrupt Configuration */
		*(REG32(SERIAL_BASE + IER_REG)) = 0x3;

		/* Switch to mode B */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0xBF;

		/* Restore EFR */
		*(REG32(SERIAL_BASE + EFR_REG)) = efr;

		/* Load Protocol Format
		 * 8 bits length
		 * 1 stop bit
		 * no parity
		 */
		*(REG32(SERIAL_BASE + LCR_REG)) = 0x3;

		/* Load new mode */
		*(REG32(SERIAL_BASE + MDR1_REG)) = 0x0;

		}	
		portEXIT_CRITICAL();

	return xReturn;
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar( xComPortHandle pxPort, signed char *pcRxedChar, portTickType xBlockTime )
{
	/* The port handle is not required as this driver only supports UART0. */
	( void ) pxPort;

	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	if( xQueueReceive( xRxedChars, pcRxedChar, xBlockTime ) )
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}
/*-----------------------------------------------------------*/

void vSerialPutString( xComPortHandle pxPort, const signed char * const pcString, unsigned short usStringLength )
{
signed char *pxNext;

	/* NOTE: This implementation does not handle the queue being full as no
	block time is used! */

	/* The port handle is not required as this driver only supports UART0. */
	( void ) pxPort;
	( void ) usStringLength;

	/* Send each character in the string, one at a time. */
	pxNext = ( signed char * ) pcString;
	while( *pxNext )
	{
		xSerialPutChar( pxPort, *pxNext, serNO_BLOCK );
		pxNext++;
	}
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar( xComPortHandle pxPort, signed char cOutChar, portTickType xBlockTime )
{
signed portBASE_TYPE xReturn;

	/* This demo driver only supports one port so the parameter is not used. */
	( void ) pxPort;
	portENTER_CRITICAL();
	{
		/* Is there space to write directly to the UART? */
		if( *plTHREEmpty == ( long ) pdTRUE )
		{
			/* We wrote the character directly to the UART, so was 
			successful. */
			*plTHREEmpty = pdFALSE;
			*(REG32(SERIAL_BASE + THR_REG)) = cOutChar;
			xReturn = pdPASS;
		}
		else 
		{
			/* We cannot write directly to the UART, so queue the character.
			Block for a maximum of xBlockTime if there is no space in the
			queue. */
			xReturn = xQueueSend( xCharsForTx, &cOutChar, xBlockTime );

			/* Depending on queue sizing and task prioritisation:  While we 
			were blocked waiting to post interrupts were not disabled.  It is 
			possible that the serial ISR has emptied the Tx queue, in which
			case we need to start the Tx off again. */
			if( ( *plTHREEmpty == ( long ) pdTRUE ) && ( xReturn == pdPASS ) )
			{
				xQueueReceive( xCharsForTx, &cOutChar, serNO_BLOCK );
				*plTHREEmpty = pdFALSE;
				*(REG32(SERIAL_BASE + THR_REG)) = cOutChar;
			}
		}
	}
	portEXIT_CRITICAL();

	return xReturn;
}
/*-----------------------------------------------------------*/

void vSerialClose( xComPortHandle xPort )
{
	/* Not supported as not required by the demo application. */
	( void ) xPort;
}
/*-----------------------------------------------------------*/





	
