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

	This file contains all the serial port components that must be compiled
	to ARM mode.  The components that can be compiled to either ARM or THUMB
	mode are contained in serial.c.

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

/* Constants to determine the ISR source. */
#define SOURCE_MODEM			((unsigned char)0x0)

#define SOURCE_THRE				((unsigned char)0x1)
#define SOURCE_RHR				((unsigned char)0x2)
#define SOURCE_RX_ERROR			((unsigned char)0x3)
#define SOURCE_RX_TIMEOUT		((unsigned char)0x6)
#define SOURCE_SPECIAL_CHAR		((unsigned char)0x8)
#define SOURCE_CTS_RTS			((unsigned char)0x10)
#define INTERRUPT_SOURCE_MASK	((unsigned char)0x3E) // we only care about bits 5:1

/* Queues used to hold received characters, and characters waiting to be
transmitted. */
static xQueueHandle xRxedChars; 
static xQueueHandle xCharsForTx; 
static volatile long lTHREEmpty;

/*-----------------------------------------------------------*/

/* 
 * The queues are created in serialISR.c as they are used from the ISR.
 * Obtain references to the queues and THRE Empty flag. 
 */
void vSerialISRCreateQueues( unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRxedChars, xQueueHandle *pxCharsForTx, long volatile **pplTHREEmptyFlag );

/* UART3 interrupt service routine entry point. */
void vUART_ISR_Wrapper( void ) __attribute__ ((naked));
/* UART3 interrupt service routine handler. */
void vUART_ISR_Handler( void ) __attribute__ ((noinline));

/*-----------------------------------------------------------*/
void vSerialISRCreateQueues(	unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRxedChars, 
								xQueueHandle *pxCharsForTx, long volatile **pplTHREEmptyFlag )
{
	/* Create the queues used to hold Rx and Tx characters. */
	xRxedChars = xQueueCreate( uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
	xCharsForTx = xQueueCreate( uxQueueLength + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );

	/* Pass back a reference to the queues so the serial API file can 
	post/receive characters. */
	*pxRxedChars = xRxedChars;
	*pxCharsForTx = xCharsForTx;

	/* Initialise the THRE empty flag - and pass back a reference. */
	lTHREEmpty = ( long ) pdTRUE;
	*pplTHREEmptyFlag = &lTHREEmpty;
}
/*-----------------------------------------------------------*/

void vUART_ISR_Wrapper( void )
{
	/* Save the context of the interrupted task. */
	portSAVE_CONTEXT();

	/* Call the handler.  This must be a separate function from the wrapper
	to ensure the correct stack frame is set up. */
	__asm volatile ("bl vUART_ISR_Handler");

	/* Restore the context of whichever task is going to run next. */
	portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

void vUART_ISR_Handler( void )
{
	
	/* Save LR. Make sure we will be able to go back to the IRQ handler */
	__asm volatile("push {lr}	\n\t");

	signed char cChar;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	volatile unsigned char *const thr = (unsigned char*)(SERIAL_BASE + THR_REG);
	volatile unsigned char *const rhr = (unsigned char*)(SERIAL_BASE + RHR_REG);
	volatile unsigned char *const lsr = (unsigned char*)(SERIAL_BASE + LSR_REG);

	/* What caused the interrupt? */
	switch( (*(REG32(SERIAL_BASE + IIR_REG)) & INTERRUPT_SOURCE_MASK) >> 1 )
	{
		case SOURCE_THRE	:	/* The THRE is empty.  If there is another
								character in the Tx queue, send it now. */
								if( xQueueReceiveFromISR( xCharsForTx, &cChar, &xHigherPriorityTaskWoken ) == pdTRUE )
								{
									
									while( (*lsr & 0x20) == 0){;}
									*thr = cChar;
								}
								else
								{
									/* There are no further characters 
									queued to send so we can indicate 
									that the THRE is available. */
									lTHREEmpty = pdTRUE;
								}
								break;

		case SOURCE_RHR	:	/* A character was received.  Place it in 
								the queue of received characters. */
								cChar = *rhr;
								xQueueSendFromISR( xRxedChars, &cChar, &xHigherPriorityTaskWoken );
								break;

		
		case SOURCE_MODEM:
		case SOURCE_RX_ERROR:
		case SOURCE_RX_TIMEOUT:
		case SOURCE_SPECIAL_CHAR:
		case SOURCE_CTS_RTS:
		default	: // unhandled interrupts
				break;
	}

	if( xHigherPriorityTaskWoken )
	{
		portYIELD_FROM_ISR();
	}

	/* Clear UART interrupt */

	*(REG32(MPU_INTC + INTC_CONTROL)) = 0x1;

	__asm volatile("pop {lr}	\n\t");

}






	
