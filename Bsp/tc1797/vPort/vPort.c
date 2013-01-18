/* Copyright 2012, Fan Wang(Parai)
 *
 * This file is part of GaInOS.
 *
 * GaInOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *             
 * Linking GaInOS statically or dynamically with other modules is making a
 * combined work based on GaInOS. Thus, the terms and conditions of the GNU
 * General Public License cover the whole combination.
 *
 * In addition, as a special exception, the copyright holders of GaInOS give
 * you permission to combine GaInOS program with free software programs or
 * libraries that are released under the GNU LGPL and with independent modules
 * that communicate with GaInOS solely through the GaInOS defined interface. 
 * You may copy and distribute such a system following the terms of the GNU GPL
 * for GaInOS and the licenses of the other code concerned, provided that you
 * include the source code of that other code when and as the GNU GPL requires
 * distribution of source code.
 *
 * Note that people who make modified versions of GaInOS are not obligated to
 * grant this special exception for their modified versions; it is their choice
 * whether to do so. The GNU General Public License gives permission to release
 * a modified version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this exception.
 * 
 * GaInOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GaInOS. If not, see <http://www.gnu.org/licenses/>.
 *
 */
/* |---------+-------------------| */
/* | Name    | GaIn OS           | */
/* |---------+-------------------| */
/* | Author: | Wang Fan(parai)   | */
/* |---------+-------------------| */
/* | Email:  | parai@foxmail.com | */
/* |---------+-------------------| */
/* | WorkOn: | Emacs23.3         | */
/* |---------+-------------------| */
#include "Kernel.h"

/* System register Definitions. */
#define vPortSYSTEM_PROGRAM_STATUS_WORD					( 0x000008FFUL ) /* Supervisor Mode, MPU Register Set 0 and Call Depth Counting disabled. */
#define vPortINITIAL_PRIVILEGED_PROGRAM_STATUS_WORD		( 0x000014FFUL ) /* IO Level 1, MPU Register Set 1 and Call Depth Counting disabled. */
#define vPortINITIAL_UNPRIVILEGED_PROGRAM_STATUS_WORD	( 0x000010FFUL ) /* IO Level 0, MPU Register Set 1 and Call Depth Counting disabled. */
#define vPortINITIAL_PCXI_UPPER_CONTEXT_WORD				( 0x00C00000UL ) /* The lower 20 bits identify the CSA address. */
#define vPortINITIAL_SYSCON								( 0x00000000UL ) /* MPU Disable. */

/* CSA manipulation macros. */
#define vPortCSA_FCX_MASK					( 0x000FFFFFUL )

/* OS Interrupt and Trap mechanisms. */
#define vPortRESTORE_PSW_MASK				( ~( 0x000000FFUL ) )
#define vPortSYSCALL_TRAP					( 6 )

/* Each CSA contains 16 words of data. */
#define vPortNUM_WORDS_IN_CSA				( 16 )
static void vPortTaskIdle(void)
{
    /* Wait Untill a task was in ready state */
    vPortEnableInterrupt();
    /* vPortSetIpl(0); */
    while(OSCurTsk == INVALID_TASK);
    vPortDispatch();
}

OsCpuIplType vPortGetIpl(void)
{

    return 0;                   /* Ommit The Compile Warning */
}

void vPortPreActivateTask(void)
{
#if(cfgOS_USE_INTERNAL_RESOURCE == STD_TRUE)
    if(OSCurTsk < cfgOS_TASK_WITH_IN_RES_NUM)
    {
        /* Do Get The Internal Resource */
        GetInResource();
    }
#endif
#if(cfgOS_PRE_TASK_HOOK == 1)
    OS_ENTER_CRITICAL();
    PreTaskHook();
    OS_EXIT_CRITICAL();
#endif
    OSTaskEntryTable[OSCurTsk]();
    OS_ASSERT(STD_FALSE);
}

void vPortSetIpl(uint8_t xIpl)
{
}
void vPortIntGetIpl(void)
{

}
void __vPortSwitch2Task(void)
{
    OSCurTsk = OSHighRdyTsk;
    if(OSCurTsk == INVALID_TASK)
    {
        vPortTaskIdle();
    }
    OSCurTcb = OSHighRdyTcb;

    if(READY == OSCurTcb->xState)
    {
        OSCurTcb->xState = RUNNING;
        vPortStartCurRdyTsk();
    }
    else
    {
      vPortRestoreSP();
      vPortRestoreContext()  
    }
}

void vPortDispatcher(void)
{
    if(RUNNING == OSCurTcb->xState || WAITING == OSCurTcb->xState)
    {
        vPortSaveContext();
        vPortSaveSP();
    }
    __asm("j __vPortSwitch2Task");
}
  
void OSTickISR(void)
{  
    vPortEnterISR();

#if(cfgOS_COUNTER_NUM >0)    
    (void)IncrementCounter(0);		/* Process the first counter,Default as system counter */
#endif
    vPortTickIsrClear();

    vPortLeaveISR();
}

