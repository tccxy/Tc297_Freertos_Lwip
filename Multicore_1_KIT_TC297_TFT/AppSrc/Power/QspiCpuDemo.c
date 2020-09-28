/**
 * \file QspiCpuDemo.c
 * \brief Qspi Master & Slave Demo (using cpu)
 *
 * \version iLLD_Demos_1_0_0_5_0
 * \copyright Copyright (c) 2014 Infineon Technologies AG. All rights reserved.
 *
 *
 *                                 IMPORTANT NOTICE
 *
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
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include <stdio.h>
#include "ConfigurationIsr.h"
#include "Ifx_Assert.h"
#include "QspiCpuDemo.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
App_QspiCpu g_QspiCpu; /**< \brief Qspi global data */

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/** \addtogroup IfxLld_Demo_QspiCpu_SrcDoc_Main_Interrupt
 * \{ */

/** \name Interrupts for Qspi Master (QSPI0) & Slave (QSPI2) drivers
 * \{ */
IFX_INTERRUPT(ISR_qspi0_Tx, 0, ISR_PRIORITY_QSPI0_TX);
IFX_INTERRUPT(ISR_qspi0_Rx, 0, ISR_PRIORITY_QSPI0_RX);
IFX_INTERRUPT(ISR_qspi0_Er, 0, ISR_PRIORITY_QSPI0_ER);
IFX_INTERRUPT(ISR_qspi2_Tx, 0, ISR_PRIORITY_QSPI2_TX);
IFX_INTERRUPT(ISR_qspi2_Rx, 0, ISR_PRIORITY_QSPI2_RX);
IFX_INTERRUPT(ISR_qspi2_Er, 0, ISR_PRIORITY_QSPI2_ER);
/** \} */

/** \} */

/** \brief Handle qspi0_Tx interrupt.
 *
 * \isrProvider \ref ISR_PROVIDER_QSPI0_TX
 * \isrPriority \ref ISR_PRIORITY_QSPI0_TX
 *
 */
void ISR_qspi0_Tx(void)
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrTransmit(&g_QspiCpu.drivers.spiMaster);
}


/** \brief Handle qspi0_Rx interrupt.
 *
 * \isrProvider \ref ISR_PROVIDER_QSPI0_RX
 * \isrPriority \ref ISR_PRIORITY_QSPI0_RX
 *
 */
void ISR_qspi0_Rx(void)
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrReceive(&g_QspiCpu.drivers.spiMaster);
}


/** \brief Handle qspi0_Er interrupt.
 *
 * \isrProvider \ref ISR_PROVIDER_QSPI0_ER
 * \isrPriority \ref ISR_PRIORITY_QSPI0_ER
 *
 */
void ISR_qspi0_Er(void)
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrError(&g_QspiCpu.drivers.spiMaster);
}


/** \brief Handle qspi2_Tx interrupt.
 *
 * \isrProvider \ref ISR_PROVIDER_QSPI2_TX
 * \isrPriority \ref ISR_PRIORITY_QSPI2_TX
 *
 */
void ISR_qspi2_Tx(void)
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrTransmit(&g_QspiCpu.drivers.spiSlave);
}


/** \brief Handle qspi2_Rx interrupt.
 *
 * \isrProvider \ref ISR_PROVIDER_QSPI2_RX
 * \isrPriority \ref ISR_PRIORITY_QSPI2_RX
 *
 */
void ISR_qspi2_Rx(void)
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrReceive(&g_QspiCpu.drivers.spiSlave);
}


/** \brief Handle qspi2_Er interrupt.
 *
 * \isrProvider \ref ISR_PROVIDER_QSPI2_ER
 * \isrPriority \ref ISR_PRIORITY_QSPI2_ER
 *
 */
void ISR_qspi2_Er(void)
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrError(&g_QspiCpu.drivers.spiSlave);
}


/** \brief Qspi Master initilisation
 *
 * This function initialises Qspi0 in master mode.
 */
static void IfxQspi_Masterinit(void)
{
    uint32                          i;
    IfxQspi_SpiMaster_Config        spiMasterConfig;
    IfxQspi_SpiMaster_ChannelConfig spiMasterChannelConfig;

    {
        /* create module config */
        IfxQspi_SpiMaster_initModuleConfig(&spiMasterConfig, &MODULE_QSPI0);

        /* set the maximum baudrate */
        spiMasterConfig.base.maximumBaudrate = 10000000;

        /* ISR priorities and interrupt target */
        spiMasterConfig.base.txPriority  = ISR_PRIORITY_QSPI0_TX;
        spiMasterConfig.base.rxPriority  = ISR_PRIORITY_QSPI0_RX;
        spiMasterConfig.base.erPriority  = ISR_PRIORITY_QSPI0_ER;
        spiMasterConfig.base.isrProvider = (IfxSrc_Tos)IfxCpu_getCoreIndex();

        /* pin configuration */
        const IfxQspi_SpiMaster_Pins pins = {&IfxQspi2_SCLK_P15_3_OUT,                               /* SCLK */
                                             IfxPort_OutputMode_pushPull,
                                             &IfxQspi2_MTSR_P15_6_OUT,  IfxPort_OutputMode_pushPull, /* MTSR */
                                             &IfxQspi2_MRSTB_P15_7_IN,  IfxPort_InputMode_pullDown,  /* MRST */
                                             IfxPort_PadDriver_cmosAutomotiveSpeed3                   /* pad driver mode */
        };
        spiMasterConfig.pins = &pins;

        /* initialize module */
        IfxQspi_SpiMaster_initModule(&g_QspiCpu.drivers.spiMaster, &spiMasterConfig);
    }

    {
        /* create channel config */
        IfxQspi_SpiMaster_initChannelConfig(&spiMasterChannelConfig,
            &g_QspiCpu.drivers.spiMaster);

        /* set the baudrate for this channel */
        spiMasterChannelConfig.base.baudrate = 5000000;

        const IfxQspi_SpiMaster_Output slsOutput = {&IfxQspi2_SLSO1_P14_2_OUT,
                                                    IfxPort_OutputMode_pushPull,
                                                    IfxPort_PadDriver_cmosAutomotiveSpeed1};

        spiMasterChannelConfig.sls.output.pin    = slsOutput.pin;
        spiMasterChannelConfig.sls.output.mode   = slsOutput.mode;
        spiMasterChannelConfig.sls.output.driver = slsOutput.driver;

        /* initialize channel */
        IfxQspi_SpiMaster_initChannel(&g_QspiCpu.drivers.spiMasterChannel,
            &spiMasterChannelConfig);
    }
#if 0
    /* init tx buffer area */
    for (i = 0; i < SPI_BUFFER_SIZE; i++)
    {
        g_QspiCpu.qspiBuffer.spi0TxBuffer[i] = (uint8)(i + 1);
        g_QspiCpu.qspiBuffer.spi2TxBuffer[i] = (uint8)(i + 100);

        g_QspiCpu.qspiBuffer.spi0RxBuffer[i] = (uint8)(0);
        g_QspiCpu.qspiBuffer.spi2RxBuffer[i] = (uint8)(0);
    }
#endif
}


/** \brief Qspi Slave initilisation
 *
 * This function initialises Qspi2 in Slave mode.
 */
static void IfxQspi_Slaveinit(void)
{
    IfxQspi_SpiSlave_Config spiSlaveConfig;

    {
        /* create module config */
        IfxQspi_SpiSlave_initModuleConfig(&spiSlaveConfig, &MODULE_QSPI2);

        /* set the maximum baudrate */
        spiSlaveConfig.base.maximumBaudrate = 10000000;

        /* ISR priorities and interrupt target */
        spiSlaveConfig.base.txPriority  = ISR_PRIORITY_QSPI2_TX;
        spiSlaveConfig.base.rxPriority  = ISR_PRIORITY_QSPI2_RX;
        spiSlaveConfig.base.erPriority  = ISR_PRIORITY_QSPI2_ER;
        spiSlaveConfig.base.isrProvider = (IfxSrc_Tos)IfxCpu_getCoreIndex();

        /* pin configuration */
        const IfxQspi_SpiSlave_Pins slavePins = {&IfxQspi2_SCLKB_P15_8_IN,                               /* SCLK Pin */
                                                 IfxPort_InputMode_pullDown,
                                                 &IfxQspi2_MTSRA_P15_5_IN,  IfxPort_InputMode_pullDown,  /* MTSR Pin */
                                                 &IfxQspi2_MRST_P15_7_OUT,  IfxPort_OutputMode_pushPull, /* MRST Pin */
                                                 &IfxQspi2_SLSIB_P15_1_IN,  IfxPort_InputMode_pullDown,  /* SLSI Pin */
                                                 IfxPort_PadDriver_cmosAutomotiveSpeed3                  /* pad driver mode */
        };
        spiSlaveConfig.pins = &slavePins;

        /* initialize module */
        IfxQspi_SpiSlave_initModule(&g_QspiCpu.drivers.spiSlave, &spiSlaveConfig);
    }
}


/** \brief Demo init API
 *
 * This function is called from main during initialization phase
 */
void IfxQspiCpuDemo_init(void)
{
    /* disable interrupts */
    boolean interruptState = IfxCpu_disableInterrupts();

    //IfxQspi_Slaveinit();
    IfxQspi_Masterinit();

    /* enable interrupts again */
    IfxCpu_restoreInterrupts(interruptState);
}


/** \brief Demo run API
 *
 * This function is called from main, background loop
 */
void IfxQspiCpuDemo_run(void)
{
#if 0
    uint32 size = SPI_BUFFER_SIZE;
    printf("Transfering %lu bytes\n", size);

    {
        {
            /* Slave transfer */
            printf("wait until slave is free\n");

            while (IfxQspi_SpiSlave_getStatus(&g_QspiCpu.drivers.spiSlave) == SpiIf_Status_busy)
            {}

            IfxQspi_SpiSlave_exchange(&g_QspiCpu.drivers.spiSlave, &g_QspiCpu.qspiBuffer.spi2TxBuffer[0],
                &g_QspiCpu.qspiBuffer.spi2RxBuffer[0], SPI_BUFFER_SIZE);

            /* Master transfer */
            printf("wait until master is free\n");

            while (IfxQspi_SpiMaster_getStatus(&g_QspiCpu.drivers.spiMasterChannel)
                   == SpiIf_Status_busy)
            {}

            IfxQspi_SpiMaster_exchange(&g_QspiCpu.drivers.spiMasterChannel, &g_QspiCpu.qspiBuffer.spi0TxBuffer[0],
                &g_QspiCpu.qspiBuffer.spi0RxBuffer[0], SPI_BUFFER_SIZE);
        }
    }

    printf("wait until slave received all data\n");

    /* wait until slave received all data */
    while (IfxQspi_SpiSlave_getStatus(&g_QspiCpu.drivers.spiSlave) == SpiIf_Status_busy)
    {}

    printf("Check received data\n");
    {
        uint32 errors       = 0;
        uint8 *spi0Received = (uint8 *)g_QspiCpu.qspiBuffer.spi0RxBuffer;
        uint8 *spi0Expected = (uint8 *)g_QspiCpu.qspiBuffer.spi2TxBuffer;
        uint8 *spi2Received = (uint8 *)g_QspiCpu.qspiBuffer.spi2RxBuffer;
        uint8 *spi2Expected = (uint8 *)g_QspiCpu.qspiBuffer.spi0TxBuffer;

        {
            uint32 i;

            for (i = 0; i < SPI_BUFFER_SIZE; i++)
            {
                if (*spi2Received++ != *spi2Expected++)
                {
                    ++errors;
                }

                if (*spi0Received++ != *spi0Expected++)
                {
                    ++errors;
                }
            }
        }

        IFX_ASSERT(IFX_VERBOSE_LEVEL_ERROR, errors == 0);

        if (errors)
        {
            printf("ERROR: received data doesn't match with expected data (%lu mismatches)\n", errors);
        }
        else
        {
            printf("OK: received data matches with expected data\n");
        }
    }
#endif
}
