#include "pub.h"
#include "TLF35584.h"

/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/
#define STM_WAIT_100USEC (10000)

/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/
extern qspiComm g_qspi;
/******************************************************************************/
/* TLF35584 Function Prototypes                                                  */
/******************************************************************************/
uint16 Tlf_ParityGeneration(uint16 Data)
{
    uint16 n = 0;

    while (Data)
    {
        Data &= Data - 1;
        ++n;
    }
    while (n > 1)
    {
        n = n - 2;
    }

    return (n);

} /*End of Tlf_ParityGeneration()*/

uint16 Tlf_PrepareSpiWriteMessage(uint8 Address, uint8 Data)
{
    uint16 SpiMsg;

    SpiMsg = ((0x8000) |                    /* Write command */
              (Address << 9) |              /* Address */
              (Data << 1));                 /* Data */
    SpiMsg |= Tlf_ParityGeneration(SpiMsg); /* Parity */

    return (SpiMsg);

} /*End of Tlf_PrepareSpiWriteMessage()*/

uint16 Tlf_PrepareSpiReadMessage(uint8 Address)
{
    uint16 SpiMsg;

    SpiMsg = ((0x0000) |                    /* Read command */
              (Address << 9));              /* Address */
    SpiMsg |= Tlf_ParityGeneration(SpiMsg); /* Parity */

    return (SpiMsg);

} /*End of Tlf_PrepareSpiReadMessage()*/

Tlf_Status Tlf_ReadRegister(uint8 Address, uint8 *RxData)
{
    uint16 SpiTxMsg;
    uint16 SpiRxMsg;
    uint16 SpiRxParityTemp;
    Tlf_Status Status = Tlf_Status_Not_Ok; /* failure on read-operation */

    /* prepare Spi read message */
    SpiTxMsg = Tlf_PrepareSpiReadMessage(Address);

    /* send Spi message */
    while (IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy)
        ;
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &SpiTxMsg, &SpiRxMsg, 1);

    /* receive Spi message */
    while (IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy)
        ;
    SpiRxParityTemp = Tlf_ParityGeneration(SpiRxMsg & 0xFFFE);
    if ((SpiRxMsg & 0x0001) == (SpiRxParityTemp))
    {
        Status = Tlf_Status_Ok;             /* successful register read operation */
        *RxData = (SpiRxMsg >> 1) & 0x00FF; /* store data byte */
    }

    return (Status);

} /*End of Tlf_ReadRegister()*/

Tlf_Status Tlf_WriteRegister(uint8 Address, uint8 TxData)
{
    uint16 SpiTxMsg;
    uint16 SpiRxMsg;
    Tlf_Status Status = Tlf_Status_Not_Ok; /* failure on write-operation */

    /* prepare Spi write message */
    SpiTxMsg = Tlf_PrepareSpiWriteMessage(Address, TxData);
    //Ifx_print("Tlf_WriteRegister  %x %x %x %x.\r\n", Address, TxData, SpiTxMsg, mm);
    while (IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy)
    {
    }

    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &SpiTxMsg,
                               &SpiRxMsg, 1);

    while (IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy)
    {
    }
    if (SpiRxMsg == SpiTxMsg)
    {
        Status = Tlf_Status_Ok; /* successful register write operation */
    }

    return (Status);

} /*End of Tlf_WriteRegister()*/

Tlf_Status Tlf_UnlockProtectedRegister(void)
{
    uint8 SpiRxData;
    Tlf_Status Status = Tlf_Status_Not_Ok; /* failure on unlock operation */

    /* UNLOCK - Send password sequence */
    Tlf_WriteRegister(PROTCFG, 0xAB);
    Tlf_WriteRegister(PROTCFG, 0xEF);
    Tlf_WriteRegister(PROTCFG, 0x56);
    Tlf_WriteRegister(PROTCFG, 0x12);

    /* need to wait for the changes to take effect */
    IfxStm_waitTicks(&MODULE_STM0, STM_WAIT_100USEC);

    /* read and check Protection status */
    Tlf_ReadRegister(PROTSTAT, &SpiRxData);
    //Ifx_print("Tlf_UnlockProtectedRegister  %x.\r\n", SpiRxData);
    if ((SpiRxData & 0x01) == 0x00)
    {
        Status = Tlf_Status_Ok; /* successful unlock operation */
    }

    return (Status);

} /*End of Tlf_UnlockProtectedRegister()*/

Tlf_Status Tlf_LockProtectedRegister(void)
{
    uint8 SpiRxData;
    Tlf_Status Status = Tlf_Status_Not_Ok; /* failure on lock operation */

    /* LOCK - Send password sequence */
    Tlf_WriteRegister(PROTCFG, 0xDF);
    Tlf_WriteRegister(PROTCFG, 0x34);
    Tlf_WriteRegister(PROTCFG, 0xBE);
    Tlf_WriteRegister(PROTCFG, 0xCA);

    /* need to wait for the changes to take effect */
    IfxStm_waitTicks(&MODULE_STM0, STM_WAIT_100USEC);

    /* read and check Protection status */
    Tlf_ReadRegister(PROTSTAT, &SpiRxData);
    //Ifx_print("Tlf_LockProtectedRegister  %x.\r\n", SpiRxData);
    if ((SpiRxData & 0x01) == 0x01)
    {
        Status = Tlf_Status_Ok; /* successful lock operation */
    }

    return (Status);

} /*End of Tlf_LockProtectedRegister()*/

/******************************************************************************/
/* Demo Function Prototypes                                                 */
/******************************************************************************/

void TLF35584Demo_ConfigSysWdCfg(void)
{

    Tlf_Status RetStatus;

    /* unlock access of protected register  */
    Tlf_UnlockProtectedRegister();

    /* send TLF35584 configuration */
    RetStatus = Tlf_WriteRegister(SYSPCFG0, 0x1);
    RetStatus = Tlf_WriteRegister(SYSPCFG1, 0x0);
    RetStatus += Tlf_WriteRegister(WDCFG0, 0x93);
    RetStatus += Tlf_WriteRegister(WDCFG1, 0x9);
    RetStatus += Tlf_WriteRegister(FWDCFG, 0xb);
    RetStatus += Tlf_WriteRegister(WWDCFG0, 0x2);
    RetStatus += Tlf_WriteRegister(WWDCFG1, 0x3);

    RetStatus += Tlf_WriteRegister(SYSSF, 0x2);
    RetStatus += Tlf_WriteRegister(IF, 0x7F);

    /* lock access of protected register  */
    Tlf_LockProtectedRegister();

} /*End of TLF35584Demo_ConfigSysWdCfg0()*/

void TLF35584_Init(void)
{
    initQSPI();

    /* configure TLF35584 System and Watchdog configuration */
    TLF35584Demo_ConfigSysWdCfg();

    /* put TLF35584 to NORMAL state */
    Tlf_WriteRegister(DEVCTRL, 0xEA);
    Tlf_WriteRegister(DEVCTRLN, 0x15);
    Ifx_print("TLF35584_Init Down .\r\n");
} /*End of TLF35584_Init()*/
