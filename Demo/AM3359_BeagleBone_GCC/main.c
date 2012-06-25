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
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 * To check the operation of the memory allocator the check task also 
 * dynamically creates a task before delaying, and deletes it again when it 
 * wakes.  If memory cannot be allocated for the new task the call to xTaskCreate
 * will fail and an error is signalled.  The dynamically created task itself
 * allocates and frees memory just to give the allocator a bit more exercise.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo application includes. */
#include "partest.h"
#include "flash.h"
#include "integer.h"
#include "PollQ.h"
#include "comtest2.h"
#include "semtest.h"
#include "flop.h"
#include "dynamic.h"
#include "BlockQ.h"
#include "serial.h"

/*-----------------------------------------------------------*/

/* Priorities for the demo application tasks. */
#define mainLED_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define mainCOM_TEST_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_POLL_PRIORITY		( tskIDLE_PRIORITY + 0 )
#define mainCHECK_TASK_PRIORITY		( tskIDLE_PRIORITY + 4 )
#define mainSEM_TEST_PRIORITY		( tskIDLE_PRIORITY + 0 )
#define mainBLOCK_Q_PRIORITY		( tskIDLE_PRIORITY + 2 )

/* The rate at which the on board LED will toggle when there is/is not an 
error. */
#define mainNO_ERROR_FLASH_PERIOD	( ( portTickType ) 3000 / portTICK_RATE_MS  )
#define mainERROR_FLASH_PERIOD		( ( portTickType ) 500 / portTICK_RATE_MS  )
#define mainON_BOARD_LED_BIT		PIN21
/* Constants used by the vMemCheckTask() task. */
#define mainCOUNT_INITIAL_VALUE		( ( unsigned long ) 0 )
#define mainNO_TASK					( 0 )

/* The size of the memory blocks allocated by the vMemCheckTask() task. */
#define mainMEM_CHECK_SIZE_1		( ( size_t ) 51 )
#define mainMEM_CHECK_SIZE_2		( ( size_t ) 52 )
#define mainMEM_CHECK_SIZE_3		( ( size_t ) 151 )

/* This is for Serial Task */
#define mainCOM_TEST_BAUD_RATE		( ( unsigned long ) 115200 )
#define mainCOM_TEST_LED			( 1 )
/*-----------------------------------------------------------*/

/*
 * The Beagleboard has 2 LEDS available on the GPIO module
 * I will use LED0 to express errors on tasks
 */

void prvToggleOnBoardLED( void );

/*
 * Configure the processor of use with the BeagleBoard
 * Currently I assume that U-boot has done all the work for us
 */
static void prvSetupHardware( void );

/*
 * Checks that all the demo application tasks are still executing without error
 * - as described at the top of the file.
 */
static long prvCheckOtherTasksAreStillRunning( unsigned long ulMemCheckTaskCount );

/*
 * The task that executes at the highest priority and calls
 * prvCheckOtherTasksAreStillRunning(). See the description at the top
 * of the file
 */
static void vErrorChecks ( void *pvParameters );

/*
 * Dynamically created and deleteted during each cycle of the vErrorChecks()
 * task. This is done to check the operation of the memory allocator.
 * See the top of vErrorChecks for more details
 */
static void vMemCheckTask( void *pvParameters );


/*-----------------------------------------------------------*/

/*
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Beableboard. */
    //	prvSetupHardware();

	// vStartLEDFlashTasks (mainLED_TASK_PRIORITY);
	// vAltStartComTestTasks( mainCOM_TEST_PRIORITY, mainCOM_TEST_BAUD_RATE, mainCOM_TEST_LED );
	// vStartIntegerMathTasks ( tskIDLE_PRIORITY );
	// vStartPolledQueueTasks ( mainQUEUE_POLL_PRIORITY );
	// vStartMathTasks ( tskIDLE_PRIORITY );
	// vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );
	// vStartDynamicPriorityTasks();
	// vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );
    //
	/* start the check task - which is defined in this file!. */

	// xTaskCreate( vErrorChecks, ( signed char *) "Check", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );

	/* Now all the tasks have been stared - start the scheduler.
	 * NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	 * Te processor MUST be in supervisor mode when vTaskStartScheduler is called.
	 * The demo applications included in the the FreeRTOS.og download swith to supervisor
	 * mode prior to main being called. If you are not using one of these demo application
	 * projects then ensure Supervisor mode is used here */

	// vTaskStartScheduler();
    
    
    /* Should never reach here! */


    //------------------------LED TOGGLE--------------------------------------
    // Writting a small test function which toggles LEDs
    
    volatile unsigned long ul;  /* volatile so it is not optimized away. */
    
    /* Initialise the LED outputs */
	prvSetupHardware();

    /* Toggle the LEDs repeatedly. */
    for( ;; )
    {
    //     /* We don't want to use the RTOS features yet, so just use a very 
    //      * crude delay mechanism instead. */
        for( ul = 0; ul < 0xfffff; ul++ )
        {
        }

    //     /* Toggle the first four LEDs (on the assumption there are at least 
    //      */
        vParTestToggleLED( 0 );
    //     // vParTestToggleLED( 1 );
        vParTestToggleLED( 2 );
    //     // vParTestToggleLED( 3 );
    }
    // //--------------------LED TOGGLE------------------------------------
    

	return 0;
}

/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{

	/* Initialize GPIOs */

    /* BONE */
    /* Enabling the GPIO1 clocks */
    (*(REG32(PRCM_REG + CM_PER_GPIO1_CLKCTRL))) =0x2;

    /* Controlling the output capability */
    (*(REG32(GPIO1_BASE+GPIO_OE))) = ~(PIN21|PIN22|PIN23|PIN24);  
 
    /* Switch off the leds */ 
    (*(REG32(GPIO1_BASE+GPIO_CLEARDATAOUT))) = PIN24|PIN23|PIN22|PIN21; 
} 


/*-----------------------------------------------------------*/

static void vErrorChecks( void *pvParameters )
{
	portTickType xDelayPeriod = mainNO_ERROR_FLASH_PERIOD;
	unsigned long ulMemCheckTaskRunningCount;
	xTaskHandle xCreatedTask;

	/* The parameters are not used in this function */
	( void ) pvParameters;
	/* Cycle for ever, delaying then checking all the other tasks are still
	 * operating without error. If an error is detected then the delay period
	 * is decreased from mainNO_ERROR_FLASH_PERIOD to mainERROR_FLASH_PERIOD
	 * so the LEDO flash rate will increase. In addition to the standard
	 * tests the memory allocator is tested through the dynamic creation and 
	 * deletion of a task each cycle. Each time the 
	 * task is created memory must be allocated for its stack. When the task
	 * is deleted this memory is returned to the heap. If the task cannot be
	 * created then it is likely that the memory allocation failed */

	for ( ;; )
	{
		/* Dynamically create a task - passing ulMemCheckTaskRunningCount
		 * as parameter */
		ulMemCheckTaskRunningCount = mainCOUNT_INITIAL_VALUE;
		xCreatedTask = mainNO_TASK;

		if( xTaskCreate( vMemCheckTask, ( signed char *) "MEM_CHECK", configMINIMAL_STACK_SIZE, ( void * ) &ulMemCheckTaskRunningCount, tskIDLE_PRIORITY, &xCreatedTask) != pdPASS ){
			/* Could not create the task - we have probably run out of heap. */
			xDelayPeriod = mainERROR_FLASH_PERIOD;
		}
		/* Delay untl it is time to execute again. */
		vTaskDelay ( xDelayPeriod );

		/* Delete the dynamically created task. */
		if( xCreatedTask != mainNO_TASK )
			vTaskDelete( xCreatedTask );
		
		/* Check all the standard demo application tasks are executing without error.
		 * ulMemCheckTaskRunningCount is checked to ensure it was modified by the task
		 * just deleted */
		if (prvCheckOtherTasksAreStillRunning( ulMemCheckTaskRunningCount ) != pdPASS)
			xDelayPeriod = mainERROR_FLASH_PERIOD;
		prvToggleOnBoardLED();
	}
}
/*----------------------------------------------------------------------------------------*/

void prvToggleOnBoardLED( void )
{
	/* Toggle LED0 */
	unsigned long ulState;
	unsigned volatile int * gpio;
    ulState = (*(REG32 (GPIO1_BASE + GPIO_DATAIN)));

	if( ulState & mainON_BOARD_LED_BIT )
	{
		gpio = (unsigned int *)(GPIO1_BASE + GPIO_SETDATAOUT);
		*gpio = mainON_BOARD_LED_BIT;
	}
	else
	{
		gpio = (unsigned int *)(GPIO1_BASE + GPIO_CLEARDATAOUT);
		*gpio = mainON_BOARD_LED_BIT;
	}
}
/*-----------------------------------------------------------------------------------------*/

static long prvCheckOtherTasksAreStillRunning( unsigned long ulMemCheckTaskCount )
{
long lReturn = ( long ) pdPASS;

	/* Check all the demo tasks (other than the flash tasks) to ensure
	that they are all still running, and that none of them have detected
	an error. */

	if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
	{
		lReturn = ( long ) pdFAIL;
	}

	/*if( xAreComTestTasksStillRunning() != pdTRUE )
	{
		lReturn = ( long ) pdFAIL;
	}*/

	if( xArePollingQueuesStillRunning() != pdTRUE )
	{
		lReturn = ( long ) pdFAIL;
	}

	if( xAreMathsTaskStillRunning() != pdTRUE )
	{
		lReturn = ( long ) pdFAIL;
	}

	if( xAreSemaphoreTasksStillRunning() != pdTRUE )
	{
		lReturn = ( long ) pdFAIL;
	}

	if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
	{
		lReturn = ( long ) pdFAIL;
	}

	if( xAreBlockingQueuesStillRunning() != pdTRUE )
	{
		lReturn = ( long ) pdFAIL;
	}

	if( ulMemCheckTaskCount == mainCOUNT_INITIAL_VALUE )
	{
		/* The vMemCheckTask did not increment the counter - it must
		have failed. */
		lReturn = ( long ) pdFAIL;
	}

	return lReturn;
}
/*--------------------------------------------------------------------------*/

static void vMemCheckTask( void *pvParameters )
{
	unsigned long *pulMemCheckTaskRunningCounter;
	void *pvMem1, *pvMem2, *pvMem3;
	static long lErrorOccurred = pdFALSE;

	/* This task is dynamically created then deleted during each cycle of the
	vErrorChecks task to check the operation of the memory allocator.  Each time
	the task is created memory is allocated for the stack and TCB.  Each time
	the task is deleted this memory is returned to the heap.  This task itself
	exercises the allocator by allocating and freeing blocks. 
	
	The task executes at the idle priority so does not require a delay. 
	
	pulMemCheckTaskRunningCounter is incremented each cycle to indicate to the
	vErrorChecks() task that this task is still executing without error. */

	pulMemCheckTaskRunningCounter = ( unsigned long * ) pvParameters;

	for( ;; )
	{
		if( lErrorOccurred == pdFALSE )
		{
			/* We have never seen an error so increment the counter. */
			( *pulMemCheckTaskRunningCounter )++;
		}

		/* Allocate some memory - just to give the allocator some extra 
		exercise.  This has to be in a critical section to ensure the
		task does not get deleted while it has memory allocated. */
		vTaskSuspendAll();
		{
			pvMem1 = pvPortMalloc( mainMEM_CHECK_SIZE_1 );
			if( pvMem1 == NULL )
			{
				lErrorOccurred = pdTRUE;
			}
			else
			{
				memset( pvMem1, 0xaa, mainMEM_CHECK_SIZE_1 );
				vPortFree( pvMem1 );
			}
		}
		xTaskResumeAll();

		/* Again - with a different size block. */
		vTaskSuspendAll();
		{
			pvMem2 = pvPortMalloc( mainMEM_CHECK_SIZE_2 );
			if( pvMem2 == NULL )
			{
				lErrorOccurred = pdTRUE;
			}
			else
			{
				memset( pvMem2, 0xaa, mainMEM_CHECK_SIZE_2 );
				vPortFree( pvMem2 );
			}
		}
		xTaskResumeAll();

		/* Again - with a different size block. */
		vTaskSuspendAll();
		{
			pvMem3 = pvPortMalloc( mainMEM_CHECK_SIZE_3 );
			if( pvMem3 == NULL )
			{
				lErrorOccurred = pdTRUE;
			}
			else
			{
				memset( pvMem3, 0xaa, mainMEM_CHECK_SIZE_3 );
				vPortFree( pvMem3 );
			}
		}
		xTaskResumeAll();
	}
}
