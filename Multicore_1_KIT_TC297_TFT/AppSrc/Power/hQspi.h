#ifndef QSPI_H_
#define QSPI_H_

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "Ifx_Types.h"
#include "IfxQspi_SpiMaster.h"
#include "IfxQspi_SpiSlave.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define SPI_BUFFER_SIZE     5                       /* Buffers size                  */

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/
/*********************************************************************************************************************/
typedef struct
{
    uint8 spiMasterTxBuffer[SPI_BUFFER_SIZE];       /* QSPI Master Transmit buffer   */
    uint8 spiMasterRxBuffer[SPI_BUFFER_SIZE];       /* QSPI Master Receive buffer    */
    uint8 spiSlaveTxBuffer[SPI_BUFFER_SIZE];        /* QSPI Slave Transmit buffer    */
    uint8 spiSlaveRxBuffer[SPI_BUFFER_SIZE];        /* QSPI Slave Receive buffer     */
} qspiBuffers;

typedef struct
{
    qspiBuffers               spiBuffers;           /* Buffers instance              */
    IfxQspi_SpiMaster         spiMaster;            /* QSPI Master handle            */
    IfxQspi_SpiMaster_Channel spiMasterChannel;     /* QSPI Master Channel handle    */
    IfxQspi_SpiSlave          spiSlave;             /* QSPI Slave handle             */
} qspiComm;

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void initPeripherals(void);
void transferData(void);
void initQSPI(void);

#endif /* QSPI_CPU_H_ */
