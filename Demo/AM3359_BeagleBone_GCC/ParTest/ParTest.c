/*
    FreeRTOS V6.1.1 - Copyright (C) 2011 Real Time Engineers Ltd.

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
	Changes from V2.5.2
		
	+ All LED's are turned off to start.
*/


#include "FreeRTOS.h"
#include "partest.h"
#include <stdlib.h>

/* Beagle board has 2 leds attched on a GPIO but I will
 * use one of them. The other one will be used for the serial I/O
 */
#define partstNUM_LEDS			( 4 )
#define partstALL_OUTPUTS_OFF	( ( unsigned long ) 0xffffffff )

/*-----------------------------------------------------------
 * Simple parallel port IO routines.
 *-----------------------------------------------------------*/


void vParTestSetLED( unsigned portBASE_TYPE uxLED, signed portBASE_TYPE xValue )
{
	unsigned int GPIO_PIN = 0;
	if( uxLED <= partstNUM_LEDS){
    /* I define LED0 (GPIO_149) as 0  and LED1 (GPIO_150) as 1 */
		switch(uxLED){
			case 0: 
				{
					GPIO_PIN=PIN21;
					break;
				}
			case 1: 
				{
					GPIO_PIN=PIN22;
					break;
				}
            case 2: 
				{
					GPIO_PIN=PIN23;
					break;
				}
            case 3: 
				{
					GPIO_PIN=PIN24;
					break;
				}

			default: break;
		};
		if ( xValue )
			//(*(REG32(GPIO5_BASE + GPIO_SETDATAOUT))) = GPIO_PIN;
            (*(REG32(GPIO1_BASE + GPIO_SETDATAOUT))) = GPIO_PIN;

		else
			//(*(REG32(GPIO5_BASE + GPIO_CLEARDATAOUT))) = GPIO_PIN;
            (*(REG32(GPIO1_BASE + GPIO_CLEARDATAOUT))) = GPIO_PIN;

	}
}

/* This function is called by Common/Minimal/flash.c (Led Task)
 * The Led Task spawns 3 tasks, each tasks handling a separate led.
 * TI Beagleboard, has only 2 leds, so the 3rd task should do nothing
 * TI BeagleBone has 4 leds
 */

void vParTestToggleLED( unsigned portBASE_TYPE uxLED )
{
	volatile unsigned long ulCurrentState, GPIO_PIN=0;
	if( uxLED <= partstNUM_LEDS )
	{
		/* Toggle LED Status
		 * LED0 = GPIO1_21
		 * LED1 = GPIO1_22
         * LED2 = GPIO1_23
         * LED3 = GPIO_24
         */

		switch(uxLED){
			case 0: 
				{
					GPIO_PIN=PIN21;
					break;
				}
			case 1: 
				{
					GPIO_PIN=PIN22;
					break;
				}
            case 2:
                {
                    GPIO_PIN=PIN23;
                }
            case 3:
                {
                    GPIO_PIN=PIN24;

                }
			default: 
				{
					// Don't do nothing
					break;
				}
		};
	}

	//ulCurrentState = (*(REG32 (GPIO5_BASE + GPIO_DATAOUT)));
    ulCurrentState = (*(REG32 (GPIO1_BASE + GPIO_DATAOUT)));
		
	/* I have to ignore the rest of the bits */
	if ( ulCurrentState & GPIO_PIN )
		(*(REG32(GPIO1_BASE + GPIO_CLEARDATAOUT))) |= GPIO_PIN;
	else
		(*(REG32(GPIO1_BASE + GPIO_SETDATAOUT))) |= GPIO_PIN;
	
}
