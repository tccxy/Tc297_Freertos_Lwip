#include "hQspi.h"
#include "IfxPort.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/* QSPI modules */
#define QSPI2_MASTER &MODULE_QSPI2 /* SPI Master module                                            */
#define QSPI3_SLAVE &MODULE_QSPI3  /* SPI Slave module                                             */

/* LED port pin */
#define LED_D110 &MODULE_P13, 3 /* LED D110 Port, Pin definition                                */

#define MASTER_CHANNEL_BAUDRATE 1000000 /* Master channel baud rate                                     */

/* Interrupt Service Routine priorities for Master and Slave SPI communication */
#define ISR_PRIORITY_MASTER_TX 50
#define ISR_PRIORITY_MASTER_RX 51
#define ISR_PRIORITY_MASTER_ER 52
#define ISR_PRIORITY_SLAVE_TX 53
#define ISR_PRIORITY_SLAVE_RX 54
#define ISR_PRIORITY_SLAVE_ER 55

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
qspiComm g_qspi;

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void initQSPI2Master(void);
void initQSPI2MasterChannel(void);
void initQSPI2MasterBuffers(void);
void initQSPI(void);
void verifyData(void);

/*********************************************************************************************************************/
/*----------------------------------------------Function Implementations---------------------------------------------*/
/*********************************************************************************************************************/
IFX_INTERRUPT(masterTxISR, 0, ISR_PRIORITY_MASTER_TX); /* SPI Master ISR for transmit data         */
IFX_INTERRUPT(masterRxISR, 0, ISR_PRIORITY_MASTER_RX); /* SPI Master ISR for receive data          */
IFX_INTERRUPT(masterErISR, 0, ISR_PRIORITY_MASTER_ER); /* SPI Master ISR for error                 */
IFX_INTERRUPT(slaveTxISR, 0, ISR_PRIORITY_SLAVE_TX);   /* SPI Slave ISR for transmit data          */
IFX_INTERRUPT(slaveRxISR, 0, ISR_PRIORITY_SLAVE_RX);   /* SPI Slave ISR for receive data           */
IFX_INTERRUPT(slaveErISR, 0, ISR_PRIORITY_SLAVE_ER);   /* SPI Slave ISR for error                  */

void masterTxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrTransmit(&g_qspi.spiMaster);
}

void masterRxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrReceive(&g_qspi.spiMaster);
}

void masterErISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrError(&g_qspi.spiMaster);
}

void slaveTxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrTransmit(&g_qspi.spiSlave);
}

void slaveRxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrReceive(&g_qspi.spiSlave);
}

void slaveErISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrError(&g_qspi.spiSlave);
}

/* QSPI Master initialization
 * This function initializes the QSPI2 module in Master mode.
 */
void initQSPI2Master(void)
{
    IfxQspi_SpiMaster_Config spiMasterConfig; /* Define a Master configuration            */

    IfxQspi_SpiMaster_initModuleConfig(&spiMasterConfig, QSPI2_MASTER); /* Initialize it with default values        */

    spiMasterConfig.base.mode = SpiIf_Mode_master; /* Configure the mode                       */

    /* Select the port pins for communication */
    const IfxQspi_SpiMaster_Pins qspi2MasterPins = {
        &IfxQspi2_SCLK_P15_3_OUT, IfxPort_OutputMode_pushPull, /* SCLK Pin                       (CLK)     */
        &IfxQspi2_MTSR_P15_6_OUT, IfxPort_OutputMode_pushPull, /* MasterTransmitSlaveReceive pin (MOSI)    */
        &IfxQspi2_MRSTB_P15_7_IN, IfxPort_InputMode_pullDown,  /* MasterReceiveSlaveTransmit pin (MISO)    */
        IfxPort_PadDriver_cmosAutomotiveSpeed3                 /* Pad driver mode                          */
    };
    spiMasterConfig.pins = &qspi2MasterPins; /* Assign the Master's port pins            */

    /* Set the ISR priorities and the service provider */
    spiMasterConfig.base.txPriority = ISR_PRIORITY_MASTER_TX;
    spiMasterConfig.base.rxPriority = ISR_PRIORITY_MASTER_RX;
    spiMasterConfig.base.erPriority = ISR_PRIORITY_MASTER_ER;
    spiMasterConfig.base.isrProvider = IfxSrc_Tos_cpu0;

    /* Initialize the QSPI Master module */
    IfxQspi_SpiMaster_initModule(&g_qspi.spiMaster, &spiMasterConfig);
}

/* QSPI Master channel initialization
 * This function initializes the QSPI2 Master channel.
 */
void initQSPI2MasterChannel(void)
{
    IfxQspi_SpiMaster_ChannelConfig spiMasterChannelConfig; /* Define a Master Channel configuration    */

    /* Initialize the configuration with default values */
    IfxQspi_SpiMaster_initChannelConfig(&spiMasterChannelConfig, &g_qspi.spiMaster);

    spiMasterChannelConfig.base.baudrate = MASTER_CHANNEL_BAUDRATE; /* Set SCLK frequency to 1 MHz              */

    /* set the transfer data width */
    spiMasterChannelConfig.base.mode.dataWidth = 16;

    spiMasterChannelConfig.base.mode.csTrailDelay = 2;
    spiMasterChannelConfig.base.mode.csInactiveDelay = 2;
    spiMasterChannelConfig.base.mode.shiftClock = SpiIf_ShiftClock_shiftTransmitDataOnTrailingEdge;
    /* Select the port pin for the Chip Select signal */
    const IfxQspi_SpiMaster_Output qspi2SlaveSelect = {
        /* QSPI2 Master selects the QSPI3 Slave     */
        &IfxQspi2_SLSO1_P14_2_OUT, IfxPort_OutputMode_pushPull, /* Slave Select port pin (CS)               */
        IfxPort_PadDriver_cmosAutomotiveSpeed1                  /* Pad driver mode                          */
    };
    spiMasterChannelConfig.sls.output = qspi2SlaveSelect;

    /* Initialize the QSPI Master channel */
    IfxQspi_SpiMaster_initChannel(&g_qspi.spiMasterChannel, &spiMasterChannelConfig);
}

/* QSPI Master SW buffer initialization
 * This function initializes SW buffers the Master uses.
 */
void initQSPI2MasterBuffers(void)
{
    for (uint8 i = 0; i < SPI_BUFFER_SIZE; i++)
    {
        g_qspi.spiBuffers.spiMasterTxBuffer[i] = (uint8)(i + 1); /* Fill TX Master Buffer with pattern       */
        g_qspi.spiBuffers.spiMasterRxBuffer[i] = 0;              /* Clear RX Buffer                          */
    }
}

/* This function initialize the QSPI modules */
void initQSPI(void)
{
    /* Firstly initialize the Slave */
    //initQSPI3Slave();
    //initQSPI3SlaveBuffers();

    /* Secondly initialize the Master */
    initQSPI2Master();
    initQSPI2MasterChannel();
    initQSPI2MasterBuffers();
}



/* This function starts the data transfer */
void transferData(void)
{
    while (IfxQspi_SpiSlave_getStatus(&g_qspi.spiSlave) == SpiIf_Status_busy ||
           IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy)
    { /* Wait until the previous communication has finished, if any */
    }

    /* Instruct the SPI Slave to receive a data stream of defined length */
    IfxQspi_SpiSlave_exchange(&g_qspi.spiSlave, NULL_PTR, &g_qspi.spiBuffers.spiSlaveRxBuffer[0], SPI_BUFFER_SIZE);

    /* Send a data stream through the SPI Master */
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], NULL_PTR, SPI_BUFFER_SIZE);

    verifyData();
}

/* This function checks if the received data is correct */
void verifyData(void)
{
    uint32 i;
    uint32 error = 0;

    /* Wait until the Slave has received all the data */
    while (IfxQspi_SpiSlave_getStatus(&g_qspi.spiSlave) == SpiIf_Status_busy)
    {
    }

    /* Check if the received data match the sent one */
    for (i = 0; i < SPI_BUFFER_SIZE; i++)
    {
        if (g_qspi.spiBuffers.spiSlaveRxBuffer[i] != g_qspi.spiBuffers.spiMasterTxBuffer[i])
        {
            error++;
        }
    }

    /* In case of no errors, turn on the LED D110 (LED is low-level active) */
    if (error == 0)
    {
        IfxPort_setPinLow(LED_D110);
    }
}
