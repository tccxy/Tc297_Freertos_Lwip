/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* TriCore specific includes. */
#include "Cpu/Std/Ifx_Types.h"
#include "Cpu/Std/IfxCpu_Intrinsics.h"
#include "Tricore/Compilers/Compilers.h"

#include "Stm/Timer/IfxStm_Timer.h"
#include "Cpu/Irq/IfxCpu_Irq.h"
#include "IfxInt_reg.h"
#include "IfxCpu_reg.h"
#include "Bsp.h"
#include "hDrv.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "list.h"

#if configCHECK_FOR_STACK_OVERFLOW > 0
#error "Stack checking cannot be used with this port, as, unlike most ports, the pxTopOfStack member of the TCB is consumed CSA.  CSA starvation, loosely equivalent to stack overflow, will result in a trap exception."
/* The stack pointer is accessible using portCSA_TO_ADDRESS( portCSA_TO_ADDRESS( pxCurrentTCB->pxTopOfStack )[ 0 ] )[ 2 ]; */
#endif /* configCHECK_FOR_STACK_OVERFLOW */

static volatile IfxStm_Timer Timer[3];
static volatile Ifx_STM *const STM[3] = {&MODULE_STM0, &MODULE_STM1, &MODULE_STM2};

volatile Ifx_SRC_SRCR *const GPSR[3] = {&SRC_GPSR_GPSR0_SR0, &SRC_GPSR_GPSR1_SR0, &SRC_GPSR_GPSR2_SR0};
/*-----------------------------------------------------------*/

/* System register Definitions. */
#define portSYSTEM_PROGRAM_STATUS_WORD (0x000008FFUL)				/* Supervisor Mode, MPU Register Set 0 and Call Depth Counting disabled. */
#define portINITIAL_PRIVILEGED_PROGRAM_STATUS_WORD (0x000014FFUL)	/* IO Level 1, MPU Register Set 1 and Call Depth Counting disabled. */
#define portINITIAL_UNPRIVILEGED_PROGRAM_STATUS_WORD (0x000010FFUL) /* IO Level 0, MPU Register Set 1 and Call Depth Counting disabled. */
#define portINITIAL_PCXI_UPPER_CONTEXT_WORD (0x00300000UL)			/* The lower 20 bits identify the CSA address. */
#define portINITIAL_SYSCON (0x00000000UL)							/* MPU Disable. */

/* CSA manipulation macros. */
#define portCSA_FCX_MASK (0x000FFFFFUL)

/* OS Interrupt and Trap mechanisms. */
#define portRESTORE_PSW_MASK (~(0x000000FFUL))

/* Each CSA contains 16 words of data. */
#define portNUM_WORDS_IN_CSA (16)

/*-----------------------------------------------------------*/

/*
 * Perform any hardware configuration necessary to generate the tick interrupt.
 */
static void prvSystemTickHandler(void);
static void prvSetupTimerInterrupt(void);

/*
 * Priority 1 interrupt handler for yields pended from an interrupt.
 */
static void prvInterruptYield(void);
/*-----------------------------------------------------------*/

/* This reference is required by the save/restore context macros. */
extern volatile unsigned long *pxCurrentTCB;

/*-----------------------------------------------------------*/

StackType_t *pxPortInitialiseStack(portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters)
{
	unsigned long *pulUpperCSA = NULL;
	unsigned long *pulLowerCSA = NULL;

	/* 16 Address Registers (4 Address registers are global), 16 Data
	Registers, and 3 System Registers.

	There are 3 registers that track the CSAs.
		FCX points to the head of globally free set of CSAs.
		PCX for the task needs to point to Lower->Upper->NULL arrangement.
		LCX points to the last free CSA so that corrective action can be taken.

	Need two CSAs to store the context of a task.
		The upper context contains D8-D15, A10-A15, PSW and PCXI->NULL.
		The lower context contains D0-D7, A2-A7, A11 and PCXI->UpperContext.
		The pxCurrentTCB->pxTopOfStack points to the Lower Context RSLCX matching the initial BISR.
		The Lower Context points to the Upper Context ready for the return from the interrupt handler.

	 The Real stack pointer for the task is stored in the A10 which is restored
	 with the upper context. */

	/* Have to disable interrupts here because the CSAs are going to be
	manipulated. */
	portENTER_CRITICAL();
	{
		/* DSync to ensure that buffering is not a problem. */
		__dsync();

		/* Consume two free CSAs. */
		pulLowerCSA = portCSA_TO_ADDRESS(__mfcr(CPU_FCX));
		if (NULL != pulLowerCSA)
		{
			/* The Lower Links to the Upper. */
			pulUpperCSA = portCSA_TO_ADDRESS(pulLowerCSA[0]);
		}

		/* Check that we have successfully reserved two CSAs. */
		if ((NULL != pulLowerCSA) && (NULL != pulUpperCSA))
		{
			/* Remove the two consumed CSAs from the free CSA list. */
			__disable();
			__dsync();
			__mtcr(CPU_FCX, pulUpperCSA[0]);
			__isync();
			__enable();
		}
		else
		{
			/* Simply trigger a context list depletion trap. */
			__svlcx();
		}
	}
	portEXIT_CRITICAL();

	/* Clear the upper CSA. */
	memset(pulUpperCSA, 0, portNUM_WORDS_IN_CSA * sizeof(unsigned long));

	/* Upper Context. */
	pulUpperCSA[2] = (unsigned long)pxTopOfStack;	 /* A10;	Stack Return aka Stack Pointer */
	pulUpperCSA[1] = portSYSTEM_PROGRAM_STATUS_WORD; /* PSW	*/

	/* Clear the lower CSA. */
	memset(pulLowerCSA, 0, portNUM_WORDS_IN_CSA * sizeof(unsigned long));

	/* Lower Context. */
	pulLowerCSA[8] = (unsigned long)pvParameters; /* A4;	Address Type Parameter Register	*/
	pulLowerCSA[1] = (unsigned long)pxCode;		  /* A11;	Return Address aka RA */

	/* PCXI pointing to the Upper context. */
	pulLowerCSA[0] = (portINITIAL_PCXI_UPPER_CONTEXT_WORD | (unsigned long)portADDRESS_TO_CSA(pulUpperCSA));

	/* Save the link to the CSA in the top of stack. */
	pxTopOfStack = (unsigned long *)portADDRESS_TO_CSA(pulLowerCSA);

	/* DSync to ensure that buffering is not a problem. */
	__dsync();

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler(void)
{
	unsigned long ulMFCR = 0UL;
	unsigned long *pulUpperCSA = NULL;
	unsigned long *pulLowerCSA = NULL;

	/* Interrupts at or below configMAX_SYSCALL_INTERRUPT_PRIORITY are disable
	when this function is called. */

	/* Set-up the timer interrupt. */
	prvSetupTimerInterrupt();

	/* Install the Trap Handlers. */

	/* Install the Syscall Handler for yield calls. */

	/* Enable then install the priority 1 interrupt for pending context
	switches from an ISR.  See mod_SRC in the TriCore manual. */
	IfxSrc_init(GPSR[CPU_ID], (IfxSrc_Tos)CPU_ID, configKERNEL_YIELD_PRIORITY);
	IfxSrc_enable(GPSR[CPU_ID]);

	__disable();

	/* Load the initial SYSCON. */
	__mtcr(CPU_SYSCON, portINITIAL_SYSCON);
	__isync();

	/* ENDINIT has already been applied in the 'cstart.c' code. */

	/* Clear the PSW.CDC to enable the use of an RFE without it generating an
	exception because this code is not genuinely in an exception. */
	ulMFCR = __mfcr(CPU_PSW);
	ulMFCR &= portRESTORE_PSW_MASK;
	__dsync();
	__mtcr(CPU_PSW, ulMFCR);
	__isync();

	/* Finally, perform the equivalent of a portRESTORE_CONTEXT() */
	pulLowerCSA = portCSA_TO_ADDRESS((*pxCurrentTCB));
	pulUpperCSA = portCSA_TO_ADDRESS(pulLowerCSA[0]);
	__dsync();
	__mtcr(CPU_PCXI, *pxCurrentTCB);
	__isync();
	__nop();
	__rslcx();
	__nop();

	/* Return to the first task selected to execute. */
	__asm volatile("rfe");

	/* Will not get here. */
	return 0;
}
/*-----------------------------------------------------------*/

static void prvSetupTimerInterrupt(void)
{
    IfxStm_Timer_Config timerConfig;
	IfxStm_Timer_initConfig(&timerConfig, STM[CPU_ID]);
	timerConfig.base.isrPriority = configKERNEL_INTERRUPT_PRIORITY;
	timerConfig.base.frequency = configTICK_RATE_HZ;

	IfxStm_Timer_init(&Timer[CPU_ID], &timerConfig);
	IfxStm_Timer_run(&Timer[CPU_ID]);
}

IFX_INTERRUPT(KERNEL_INTERRUPT, 0, configKERNEL_INTERRUPT_PRIORITY);

void KERNEL_INTERRUPT(void)
{
	prvSystemTickHandler();
}


//__attribute__((noinline))
static void prvSystemTickHandler(void)
{
	unsigned long ulSavedInterruptMask;
	unsigned long *pxUpperCSA = NULL;
	unsigned long xUpperCSA = 0UL;
	extern volatile unsigned long *pxCurrentTCB;
	long lYieldRequired;

	/* Reload the Compare Match register for X ticks into the future.

 	 If critical section or interrupt nesting budgets are exceeded, then
	 it is possible that the calculated next compare match value is in the
	 past.  If this occurs (unlikely), it is possible that the resulting
	 time slippage will exceed a single tick period.  Any adverse effect of
	 this is time bounded by the fact that only the first n bits of the 56 bit
	 STM timer are being used for a compare match, so another compare match
	 will occur after an overflow in just those n bits (not the entire 56 bits).
	 As an example, if the peripheral clock is 75MHz, and the tick rate is 1KHz,
	 a missed tick could result in the next tick interrupt occurring within a
	 time that is 1.7 times the desired period.  The fact that this is greater
	 than a single tick period is an effect of using a timer that cannot be
	 automatically reset, in hardware, by the occurrence of a tick interrupt.
	 Changing the tick source to a timer that has an automatic reset on compare
	 match (such as a GPTA timer) will reduce the maximum possible additional
	 period to exactly 1 times the desired period. */

	IfxStm_Timer_acknowledgeTimerIrq(&Timer[CPU_ID]);

	/* Kernel API calls require Critical Sections. */
	ulSavedInterruptMask = portSET_INTERRUPT_MASK_FROM_ISR();
	{
		/* Increment the Tick. */
		lYieldRequired = xTaskIncrementTick();
	}
	portCLEAR_INTERRUPT_MASK_FROM_ISR(ulSavedInterruptMask);

	if (lYieldRequired != pdFALSE)
#if configUSE_PREEMPTION == 1
	{
		/* Save the context of a task.
   	   The upper context is automatically saved when entering a trap or interrupt.
	   Need to save the lower context as well and copy the PCXI CSA ID into
	   pxCurrentTCB->pxTopOfStack. Only Lower Context CSA IDs may be saved to the
	   TCB of a task.

	   Call vTaskSwitchContext to select the next task, note that this changes the
	   value of pxCurrentTCB so that it needs to be reloaded.

	   Call vPortSetMPURegisterSetOne to change the MPU mapping for the task
	   that has just been switched in.

	   Load the context of the task.
	   Need to restore the lower context by loading the CSA from
	   pxCurrentTCB->pxTopOfStack into PCXI (effectively changing the call stack).
	   In the Interrupt handler post-amble, RSLCX will restore the lower context
	   of the task. RFE will restore the upper context of the task, jump to the
	   return address and restore the previous state of interrupts being
	   enabled/disabled. */
		__disable();
		__dsync();
		xUpperCSA = __mfcr(CPU_PCXI);
		pxUpperCSA = portCSA_TO_ADDRESS(xUpperCSA);
		*pxCurrentTCB = pxUpperCSA[0];
		vTaskSwitchContext();
		pxUpperCSA[0] = *pxCurrentTCB;
		GPSR[CPU_ID]->B.SETR = 0;
		__isync();
	}
#endif
}

/*-----------------------------------------------------------*/

/*
 * When a task is deleted, it is yielded permanently until the IDLE task
 * has an opportunity to reclaim the memory that that task was using.
 * Typically, the memory used by a task is the TCB and Stack but in the
 * TriCore this includes the CSAs that were consumed as part of the Call
 * Stack. These CSAs can only be returned to the Globally Free Pool when
 * they are not part of the current Call Stack, hence, delaying the
 * reclamation until the IDLE task is freeing the task's other resources.
 * This function uses the head of the linked list of CSAs (from when the
 * task yielded for the last time) and finds the tail (the very bottom of
 * the call stack) and inserts this list at the head of the Free list,
 * attaching the existing Free List to the tail of the reclaimed call stack.
 *
 * NOTE: the IDLE task needs processing time to complete this function
 * and in heavily loaded systems, the Free CSAs may be consumed faster
 * than they can be freed assuming that tasks are being spawned and
 * deleted frequently.
 */
void vPortReclaimCSA(unsigned long *pxTCB)
{
	unsigned long pxHeadCSA, pxTailCSA, pxFreeCSA;
	unsigned long *pulNextCSA;

	/* A pointer to the first CSA in the list of CSAs consumed by the task is
	stored in the first element of the tasks TCB structure (where the stack
	pointer would be on a traditional stack based architecture). */
	pxHeadCSA = (*pxTCB) & portCSA_FCX_MASK;

	/* Mask off everything in the CSA link field other than the address.  If
	the	address is NULL, then the CSA is not linking anywhere and there is
	nothing	to do. */
	pxTailCSA = pxHeadCSA;

	/* Convert the link value to contain just a raw address and store this
	in a local variable. */
	pulNextCSA = portCSA_TO_ADDRESS(pxTailCSA);

	/* Iterate over the CSAs that were consumed as part of the task.  The
	first field in the CSA is the pointer to then next CSA.  Mask off
	everything in the pointer to the next CSA, other than the link address.
	If this is NULL, then the CSA currently being pointed to is the last in
	the chain. */
	while (0UL != (pulNextCSA[0] & portCSA_FCX_MASK))
	{
		/* Clear all bits of the pointer to the next in the chain, other
		than the address bits themselves. */
		pulNextCSA[0] = pulNextCSA[0] & portCSA_FCX_MASK;

		/* Move the pointer to point to the next CSA in the list. */
		pxTailCSA = pulNextCSA[0];

		/* Update the local pointer to the CSA. */
		pulNextCSA = portCSA_TO_ADDRESS(pxTailCSA);
	}

	__disable();
	{
		/* Look up the current free CSA head. */
		__dsync();
		pxFreeCSA = __mfcr(CPU_FCX);

		/* Join the current Free onto the Tail of what is being reclaimed. */
		portCSA_TO_ADDRESS(pxTailCSA)[0] = pxFreeCSA;

		/* Move the head of the reclaimed into the Free. */
		__dsync();
		__mtcr(CPU_FCX, pxHeadCSA);
		__isync();
	}
	__enable();
}
/*-----------------------------------------------------------*/

void vPortEndScheduler(void)
{
	/* Nothing to do. Unlikely to want to end. */
}
/*-----------------------------------------------------------*/

void prvTrapYield(int iTrapIdentification)
{
	unsigned long *pxUpperCSA = NULL;
	unsigned long xUpperCSA = 0UL;
	extern volatile unsigned long *pxCurrentTCB;

	switch (iTrapIdentification)
	{
	case portSYSCALL_TASK_YIELD:
		/* Save the context of a task.
	 	 The upper context is automatically saved when entering a trap or interrupt.
		 Need to save the lower context as well and copy the PCXI CSA ID into
		 pxCurrentTCB->pxTopOfStack. Only Lower Context CSA IDs may be saved to the
		 TCB of a task.

		 Call vTaskSwitchContext to select the next task, note that this changes the
		 value of pxCurrentTCB so that it needs to be reloaded.

		 Call vPortSetMPURegisterSetOne to change the MPU mapping for the task
		 that has just been switched in.

		 Load the context of the task.
		 Need to restore the lower context by loading the CSA from
		 pxCurrentTCB->pxTopOfStack into PCXI (effectively changing the call stack).
		 In the Interrupt handler post-amble, RSLCX will restore the lower context
		 of the task. RFE will restore the upper context of the task, jump to the
		 return address and restore the previous state of interrupts being
		 enabled/disabled. */
		__disable();
		__dsync();
		xUpperCSA = __mfcr(CPU_PCXI);
		pxUpperCSA = portCSA_TO_ADDRESS(xUpperCSA);
		*pxCurrentTCB = pxUpperCSA[0];
		vTaskSwitchContext();
		pxUpperCSA[0] = *pxCurrentTCB;
		GPSR[CPU_ID]->B.SETR = 0;
		__isync();
		break;

	default:
		/* Unimplemented trap called. */
		configASSERT(((volatile void *)NULL));
		break;
	}
}
/*-----------------------------------------------------------*/

IFX_INTERRUPT(KERNEL_YIELD, 0, configKERNEL_YIELD_PRIORITY);
void KERNEL_YIELD(void)
{
	prvInterruptYield();
}

__attribute__((noinline)) static void prvInterruptYield(void)
{
	unsigned long *pxUpperCSA = NULL;
	unsigned long xUpperCSA = 0UL;
	extern volatile unsigned long *pxCurrentTCB;

	/* Save the context of a task.
     The upper context is automatically saved when entering a trap or interrupt.
	 Need to save the lower context as well and copy the PCXI CSA ID into
	 pxCurrentTCB->pxTopOfStack. Only Lower Context CSA IDs may be saved to the
	 TCB of a task.

	 Call vTaskSwitchContext to select the next task, note that this changes the
	 value of pxCurrentTCB so that it needs to be reloaded.

	 Call vPortSetMPURegisterSetOne to change the MPU mapping for the task
	 that has just been switched in.

	 Load the context of the task.
	 Need to restore the lower context by loading the CSA from
	 pxCurrentTCB->pxTopOfStack into PCXI (effectively changing the call stack).
	 In the Interrupt handler post-amble, RSLCX will restore the lower context
	 of the task. RFE will restore the upper context of the task, jump to the
	 return address and restore the previous state of interrupts being
	 enabled/disabled. */
	__disable();
	__dsync();
	xUpperCSA = __mfcr(CPU_PCXI);
	pxUpperCSA = portCSA_TO_ADDRESS(xUpperCSA);
	*pxCurrentTCB = pxUpperCSA[0];
	vTaskSwitchContext();
	pxUpperCSA[0] = *pxCurrentTCB;
	GPSR[CPU_ID]->B.SETR = 0;
	__isync();
}

/*-----------------------------------------------------------*/

unsigned long uxPortSetInterruptMaskFromISR(void)
{
	unsigned long uxReturn = 0UL;

	__disable();
	uxReturn = __mfcr(CPU_ICR);
	__mtcr(CPU_ICR, ((uxReturn & ~portCCPN_MASK) | configMAX_SYSCALL_INTERRUPT_PRIORITY));
	__isync();
	__enable();

	/* Return just the interrupt mask bits. */
	return (uxReturn & portCCPN_MASK);
}
/*-----------------------------------------------------------*/
