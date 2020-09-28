/**
 * \file QspiDemo.c
 * \brief Demo main file
 *
 * \copyright Copyright (c) 2013 Infineon Technologies AG. All rights reserved.
 *
 *                                 IMPORTANT NOTICE
 *
 * Infineon Technologies AG (Infineon) is supplying this file for use
 * exclusively with Infineon's microcontroller products. This file can be freely
 * distributed within development tools that are supporting such microcontroller
 * products.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "Ifx_Types.h"
#include "IfxCpu_Intrinsics.h"
#include "IfxScuWdt.h"
#include "IfxPort.h"
#include "IfxQspi_SpiMaster.h"

#include "QspiDemo.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/
//#define DMA_USE

/* interrupt priorities */
#define IFX_INTPRIO_QSPI2_TX  	(1)
#define IFX_INTPRIO_QSPI2_RX  	(2)
#define IFX_INTPRIO_QSPI2_ER  	(5)

/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/
IfxQspi_SpiMaster Qspi;
IfxQspi_SpiMaster_Channel QspiChannel;


/******************************************************************************/
/* Interrupt Function Prototypes                                              */
/******************************************************************************/
    IFX_INTERRUPT(QSPI2TxISR, 0, IFX_INTPRIO_QSPI2_TX )
    {
        IfxQspi_SpiMaster_isrTransmit(&Qspi);
    }
    IFX_INTERRUPT(QSPI2RxISR, 0, IFX_INTPRIO_QSPI2_RX)
    {
        IfxQspi_SpiMaster_isrReceive(&Qspi);
    }
    IFX_INTERRUPT(QSPI2ErISR, 0, IFX_INTPRIO_QSPI2_ER)
    {
        IfxQspi_SpiMaster_isrError(&Qspi);
    }



/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
void SpiMasterModule_Init(void)
{
    /* create module configuration */
    IfxQspi_SpiMaster_Config spiMasterConfig;
    IfxQspi_SpiMaster_initModuleConfig(&spiMasterConfig, &MODULE_QSPI2);

    /* set the desired mode and maximum baud rate */
    spiMasterConfig.base.mode             = SpiIf_Mode_master;
    spiMasterConfig.base.maximumBaudrate  = 10000000;

    /* ISR priorities and interrupt target */
    spiMasterConfig.base.txPriority       = IFX_INTPRIO_QSPI2_TX;
    spiMasterConfig.base.rxPriority       = IFX_INTPRIO_QSPI2_RX;
    spiMasterConfig.base.erPriority       = IFX_INTPRIO_QSPI2_ER;

    /* pin configuration */
    const IfxQspi_SpiMaster_Pins pins = {
      &IfxQspi2_SCLK_P15_3_OUT, IfxPort_OutputMode_pushPull, /* SCLK */
      &IfxQspi2_MTSR_P15_6_OUT, IfxPort_OutputMode_pushPull, /* MTSR */
      &IfxQspi2_MRSTB_P15_7_IN, IfxPort_InputMode_pullDown,  /* MRST */
      IfxPort_PadDriver_cmosAutomotiveSpeed1 /* pad driver mode */
    };
    spiMasterConfig.pins  = &pins;

    spiMasterConfig.base.isrProvider  = (IfxSrc_Tos)IfxCpu_getCoreId();

    /* initialize module */
    IfxQspi_SpiMaster_initModule(&Qspi, &spiMasterConfig);

}


void SpiMasterChannel_Init(void)
{
    /* create channel configuration */
    IfxQspi_SpiMaster_ChannelConfig spiMasterChannelConfig;
    IfxQspi_SpiMaster_initChannelConfig(&spiMasterChannelConfig, &Qspi);

    /* set the baud rate for this channel */
    spiMasterChannelConfig.base.baudrate             = 5000000;
    spiMasterChannelConfig.base.mode.dataWidth       = 16;
    spiMasterChannelConfig.base.mode.shiftClock      = SpiIf_ShiftClock_shiftTransmitDataOnTrailingEdge;
    spiMasterChannelConfig.base.mode.csLeadDelay     = SpiIf_SlsoTiming_7;
    spiMasterChannelConfig.base.mode.csTrailDelay    = SpiIf_SlsoTiming_7;
    spiMasterChannelConfig.base.mode.csInactiveDelay = SpiIf_SlsoTiming_7;

    /* select pin configuration */
    const IfxQspi_SpiMaster_Output slsOutput = {
      &IfxQspi2_SLSO1_P14_2_OUT,
      IfxPort_OutputMode_pushPull,
      IfxPort_PadDriver_cmosAutomotiveSpeed1
    };
    spiMasterChannelConfig.sls.output = slsOutput;

    /* initialize channel */
    IfxQspi_SpiMaster_initChannel(&QspiChannel, &spiMasterChannelConfig);

}


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
void QspiDemo_Init(void)
{
    /* configure Qspi for TLF35584 device */
    SpiMasterModule_Init();
    SpiMasterChannel_Init();

}

