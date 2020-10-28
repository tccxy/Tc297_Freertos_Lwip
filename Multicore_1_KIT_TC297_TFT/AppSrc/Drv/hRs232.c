#include "hDrv.h"
#include "Pub.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define UART_BAUDRATE 115200 /* UART baud rate in bit/s                  */

#define RS232_0_RX IfxAsclin1_RXF_P33_13_IN /* UART receive port pin                    */
#define RS232_0_TX IfxAsclin1_TX_P33_12_OUT /* UART transmit port pin                   */

#define RS232_1_RX IfxAsclin2_RXB_P02_1_IN /* UART receive port pin                    */
#define RS232_1_TX IfxAsclin2_TX_P02_0_OUT /* UART transmit port pin                   */

/* Definition of the interrupt priorities */
#define ISR_PRIORITY_RS232_0_RX 23
#define ISR_PRIORITY_RS232_0_TX 22

#define ISR_PRIORITY_RS232_1_RX 25
#define ISR_PRIORITY_RS232_1_TX 24

#define UART_RX_BUFFER_SIZE 64 /* Definition of the receive buffer size    */
#define UART_TX_BUFFER_SIZE 64 /* Definition of the transmit buffer size   */
#define SIZE 12                /* Size of the string                       */

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
/* Declaration of the ASC handle */
static IfxAsclin_Asc g_rs232_Handle[2];

/* Declaration of the FIFOs parameters */
static uint8 g_ascTxBuffer[UART_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];
static uint8 g_ascRxBuffer[UART_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + 8];

/* Definition of txData and rxData */
uint8 g_txData[] = "Hello World!";
uint8 g_rxData[SIZE] = {''};

/* Size of the message */
Ifx_SizeT g_count = sizeof(g_txData);

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/* Adding of the interrupt service routines */
IFX_INTERRUPT(rs232_0_TxISR, 0, ISR_PRIORITY_RS232_0_TX);
void rs232_0_TxISR(void)
{
    IfxAsclin_Asc_isrTransmit(&g_rs232_Handle[0]);
}

IFX_INTERRUPT(rs232_0_RxISR, 0, ISR_PRIORITY_RS232_0_RX);
void rs232_0_RxISR(void)
{
    IfxAsclin_Asc_isrReceive(&g_rs232_Handle[0]);
}

IFX_INTERRUPT(rs232_1_TxISR, 0, ISR_PRIORITY_RS232_1_TX);
void rs232_1_TxISR(void)
{
    IfxAsclin_Asc_isrTransmit(&g_rs232_Handle[1]);
}

IFX_INTERRUPT(rs232_1_RxISR, 0, ISR_PRIORITY_RS232_1_RX);
void rs232_1_RxISR(void)
{
    IfxAsclin_Asc_isrReceive(&g_rs232_Handle[1]);
}

/* This function initializes the ASCLIN UART module */
void init_rs232_0(void)
{
    /* Initialize an instance of IfxAsclin_Asc_Config with default values */
    IfxAsclin_Asc_Config ascConfig;
    IfxAsclin_Asc_initModuleConfig(&ascConfig, &MODULE_ASCLIN1);

    /* Set the desired baud rate */
    ascConfig.baudrate.baudrate = UART_BAUDRATE;

    /* ISR priorities and interrupt target */
    ascConfig.interrupt.txPriority = ISR_PRIORITY_RS232_0_TX;
    ascConfig.interrupt.rxPriority = ISR_PRIORITY_RS232_0_RX;
    ascConfig.interrupt.typeOfService = IfxCpu_Irq_getTos(IfxCpu_getCoreIndex());

    /* FIFO configuration */
    ascConfig.txBuffer = &g_ascTxBuffer;
    ascConfig.txBufferSize = UART_TX_BUFFER_SIZE;
    ascConfig.rxBuffer = &g_ascRxBuffer;
    ascConfig.rxBufferSize = UART_RX_BUFFER_SIZE;

    /* Pin configuration */
    const IfxAsclin_Asc_Pins pins =
        {
            NULL_PTR, IfxPort_InputMode_pullUp,       /* CTS pin not used */
            &RS232_0_RX, IfxPort_InputMode_pullUp,    /* RX pin           */
            NULL_PTR, IfxPort_OutputMode_pushPull,    /* RTS pin not used */
            &RS232_0_TX, IfxPort_OutputMode_pushPull, /* TX pin           */
            IfxPort_PadDriver_cmosAutomotiveSpeed1};
    ascConfig.pins = &pins;

    IfxAsclin_Asc_initModule(&g_rs232_Handle[0], &ascConfig); /* Initialize module with above parameters */
    Ifx_print("init_rs232_0 done.\r\n");
}

void init_rs232_1(void)
{
    /* Initialize an instance of IfxAsclin_Asc_Config with default values */
    IfxAsclin_Asc_Config ascConfig;
    IfxAsclin_Asc_initModuleConfig(&ascConfig, &MODULE_ASCLIN2);

    /* Set the desired baud rate */
    ascConfig.baudrate.baudrate = UART_BAUDRATE;

    /* ISR priorities and interrupt target */
    ascConfig.interrupt.txPriority = ISR_PRIORITY_RS232_1_TX;
    ascConfig.interrupt.rxPriority = ISR_PRIORITY_RS232_1_RX;
    ascConfig.interrupt.typeOfService = IfxCpu_Irq_getTos(IfxCpu_getCoreIndex());

    /* FIFO configuration */
    ascConfig.txBuffer = &g_ascTxBuffer;
    ascConfig.txBufferSize = UART_TX_BUFFER_SIZE;
    ascConfig.rxBuffer = &g_ascRxBuffer;
    ascConfig.rxBufferSize = UART_RX_BUFFER_SIZE;

    /* Pin configuration */
    const IfxAsclin_Asc_Pins pins =
        {
            NULL_PTR, IfxPort_InputMode_pullUp,       /* CTS pin not used */
            &RS232_1_RX, IfxPort_InputMode_pullUp,    /* RX pin           */
            NULL_PTR, IfxPort_OutputMode_pushPull,    /* RTS pin not used */
            &RS232_1_TX, IfxPort_OutputMode_pushPull, /* TX pin           */
            IfxPort_PadDriver_cmosAutomotiveSpeed1};
    ascConfig.pins = &pins;

    IfxAsclin_Asc_initModule(&g_rs232_Handle[1], &ascConfig); /* Initialize module with above parameters */
    Ifx_print("init_rs232_1 done.\r\n");
}

/* This function sends and receives the string "Hello World!" */
void rs232_0_send(void)
{
    IfxAsclin_Asc_write(&g_rs232_Handle[0], g_txData, &g_count, TIME_INFINITE); /* Transmit data via TX */
    //IfxAsclin_Asc_read(&g_rs232_Handle, g_rxData, &g_count, TIME_INFINITE);    /* Receive data via RX  */
}

void rs232_1_send(void)
{
    IfxAsclin_Asc_write(&g_rs232_Handle[1], g_txData, &g_count, TIME_INFINITE); /* Transmit data via TX */
    //IfxAsclin_Asc_read(&g_rs232_Handle, g_rxData, &g_count, TIME_INFINITE);    /* Receive data via RX  */
}
