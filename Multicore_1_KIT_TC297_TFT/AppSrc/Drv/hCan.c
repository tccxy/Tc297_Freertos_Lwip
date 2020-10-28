#include "hDrv.h"
#include "Pub.h"

#define CAN_MESSAGE_ID 0x777 /* Message ID that will be used in arbitration phase */

#define INVALID_DATA_VALUE (uint32)0xDEADBEEF /* Used to invalidate RX message data content        */
#define INVALID_ID_VALUE (uint32)0xFFFFFFFF   /* Used to invalidate RX message ID value            */

#define SEND_MESSAGE_OBJECT_ID_0 (IfxMultican_MsgObjId)0 /* Source message object ID                          */
#define RECV_MESSAGE_OBJECT_ID_0 (IfxMultican_MsgObjId)1 /* Destination message object ID                     */
#define ISR_PRIORITY_CAN_TX_0 11                         /* Define the CAN TX interrupt priority              */
#define ISR_PRIORITY_CAN_RX_0 10                         /* Define the CAN RX interrupt priority              */

#define ISR_PRIORITY_CAN_TX_1 13 /* Define the CAN TX interrupt priority              */
#define ISR_PRIORITY_CAN_RX_1 12 /* Define the CAN RX interrupt priority              */

#define ISR_PRIORITY_CAN_TX_2 15 /* Define the CAN TX interrupt priority              */
#define ISR_PRIORITY_CAN_RX_2 14 /* Define the CAN RX interrupt priority              */

#define ISR_PRIORITY_CAN_TX_3 17 /* Define the CAN TX interrupt priority              */
#define ISR_PRIORITY_CAN_RX_3 16 /* Define the CAN RX interrupt priority              */

struct drv_multi_can g_drv_multi_can;

IFX_INTERRUPT(canIsrTxHandler_0, 0, ISR_PRIORITY_CAN_TX_0);
IFX_INTERRUPT(canIsrRxHandler_0, 0, ISR_PRIORITY_CAN_RX_0);
IFX_INTERRUPT(canIsrTxHandler_1, 0, ISR_PRIORITY_CAN_TX_1);
IFX_INTERRUPT(canIsrRxHandler_1, 0, ISR_PRIORITY_CAN_RX_1);
IFX_INTERRUPT(canIsrTxHandler_2, 0, ISR_PRIORITY_CAN_TX_2);
IFX_INTERRUPT(canIsrRxHandler_2, 0, ISR_PRIORITY_CAN_RX_2);
IFX_INTERRUPT(canIsrTxHandler_3, 0, ISR_PRIORITY_CAN_TX_3);
IFX_INTERRUPT(canIsrRxHandler_3, 0, ISR_PRIORITY_CAN_RX_3);
void canIsrTxHandler_0(void)
{
    /* Just to indicate that the CAN message has been transmitted by turning on LED1 */
    //Ifx_print("canIsrTxHandler_0 \r\n");
}

int flag = 0;
int led_state = 0;
void canIsrRxHandler_0(void)
{
    IfxMultican_Status readStatus;
    //Ifx_print("canIsrRxHandler_0 .....\r\n");
    /* Read the received CAN message and store the status of the operation */
    readStatus = IfxMultican_Can_MsgObj_readMessage(&g_drv_multi_can.multi_can_node[0].can_rcv_msgobj,
                                                    &g_drv_multi_can.multi_can_node[0].can_rx_msg);

    /* If no new data has been received, report an error */
    if (!(readStatus & IfxMultican_Status_newData))
    {
        while (1)
        {
        }
    }

    /* If new data has been received but with one message lost, report an error */
    if (readStatus == IfxMultican_Status_newDataButOneLost)
    {
        while (1)
        {
        }
    }
    flag++;

    if (led_state == 0)
    {
        led_107_on();
        led_state = 1;
    }
    else
    {
        led_107_off();
        led_state = 0;
    }

    flag = 1;
}

void canIsrTxHandler_1(void)
{
}

void canIsrRxHandler_1(void)
{
    IfxMultican_Status readStatus;
    readStatus = IfxMultican_Can_MsgObj_readMessage(&g_drv_multi_can.multi_can_node[1].can_rcv_msgobj,
                                                    &g_drv_multi_can.multi_can_node[1].can_rx_msg);
    if (!(readStatus & IfxMultican_Status_newData))
    {
        while (1)
        {
        }
    }
    if (readStatus == IfxMultican_Status_newDataButOneLost)
    {
        while (1)
        {
        }
    }
}

void canIsrTxHandler_2(void)
{
}

void canIsrRxHandler_2(void)
{
    IfxMultican_Status readStatus;
    readStatus = IfxMultican_Can_MsgObj_readMessage(&g_drv_multi_can.multi_can_node[2].can_rcv_msgobj,
                                                    &g_drv_multi_can.multi_can_node[2].can_rx_msg);
    if (!(readStatus & IfxMultican_Status_newData))
    {
        while (1)
        {
        }
    }
    if (readStatus == IfxMultican_Status_newDataButOneLost)
    {
        while (1)
        {
        }
    }
}

void canIsrTxHandler_3(void)
{
}

void canIsrRxHandler_3(void)
{
    IfxMultican_Status readStatus;
    readStatus = IfxMultican_Can_MsgObj_readMessage(&g_drv_multi_can.multi_can_node[3].can_rcv_msgobj,
                                                    &g_drv_multi_can.multi_can_node[3].can_rx_msg);
    if (!(readStatus & IfxMultican_Status_newData))
    {
        while (1)
        {
        }
    }
    if (readStatus == IfxMultican_Status_newDataButOneLost)
    {
        while (1)
        {
        }
    }
}
/***************************************************************************
 *                 Init                                                    *
 * ************************************************************************/
void multi_can_node0_init(void)
{
    IfxMultican_Can_Node_initConfig(&g_drv_multi_can.multi_can_node[0].can_node_config, &g_drv_multi_can.can);

    g_drv_multi_can.multi_can_node[0].can_node_config.nodeId = IfxMultican_NodeId_0;

    g_drv_multi_can.multi_can_node[0].can_node_config.rxPin = &IfxMultican_RXD0B_P20_7_IN;
    g_drv_multi_can.multi_can_node[0].can_node_config.rxPinMode = IfxPort_InputMode_pullUp;
    g_drv_multi_can.multi_can_node[0].can_node_config.txPin = &IfxMultican_TXD0_P20_8_OUT;
    g_drv_multi_can.multi_can_node[0].can_node_config.txPinMode = IfxPort_OutputMode_pushPull;

    //node 0
    IfxMultican_Can_Node_init(&g_drv_multi_can.multi_can_node[0].can_node, &g_drv_multi_can.multi_can_node[0].can_node_config);

    //send msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_can.multi_can_node[0].can_msg_obj_config,
                                      &g_drv_multi_can.multi_can_node[0].can_node);

    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.msgObjId = SEND_MESSAGE_OBJECT_ID_0;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.messageId = CAN_MESSAGE_ID;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.frame = IfxMultican_Frame_transmit;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.txInterrupt.enabled = TRUE;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.txInterrupt.srcId = IfxMultican_SrcId_0;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_1;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.control.matchingId = TRUE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_can.multi_can_node[0].can_send_msgobj,
                                &g_drv_multi_can.multi_can_node[0].can_msg_obj_config);
    //rcv msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_can.multi_can_node[0].can_msg_obj_config,
                                      &g_drv_multi_can.multi_can_node[0].can_node);

    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.msgObjId = RECV_MESSAGE_OBJECT_ID_0;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.messageId = 0;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.frame = IfxMultican_Frame_receive;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.rxInterrupt.enabled = TRUE;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.txInterrupt.srcId = IfxMultican_SrcId_0;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_1;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.acceptanceMask = 0;
    g_drv_multi_can.multi_can_node[0].can_msg_obj_config.control.matchingId = TRUE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_can.multi_can_node[0].can_rcv_msgobj,
                                &g_drv_multi_can.multi_can_node[0].can_msg_obj_config);
    Ifx_print("init_multi_can_node 0 done.\r\n");
}

void multi_can_node1_init(void)
{
    IfxMultican_Can_Node_initConfig(&g_drv_multi_can.multi_can_node[1].can_node_config, &g_drv_multi_can.can);

    g_drv_multi_can.multi_can_node[1].can_node_config.nodeId = IfxMultican_NodeId_1;

    g_drv_multi_can.multi_can_node[1].can_node_config.rxPin = &IfxMultican_RXD1D_P00_1_IN;
    g_drv_multi_can.multi_can_node[1].can_node_config.rxPinMode = IfxPort_InputMode_pullUp;
    g_drv_multi_can.multi_can_node[1].can_node_config.txPin = &IfxMultican_TXD1_P00_0_OUT;
    g_drv_multi_can.multi_can_node[1].can_node_config.txPinMode = IfxPort_OutputMode_pushPull;

    //node 0
    IfxMultican_Can_Node_init(&g_drv_multi_can.multi_can_node[1].can_node, &g_drv_multi_can.multi_can_node[1].can_node_config);

    //send msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_can.multi_can_node[1].can_msg_obj_config,
                                      &g_drv_multi_can.multi_can_node[1].can_node);

    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.msgObjId = SEND_MESSAGE_OBJECT_ID_0;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.messageId = CAN_MESSAGE_ID;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.frame = IfxMultican_Frame_transmit;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.txInterrupt.enabled = TRUE;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.txInterrupt.srcId = IfxMultican_SrcId_2;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_3;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.control.matchingId = TRUE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_can.multi_can_node[1].can_send_msgobj,
                                &g_drv_multi_can.multi_can_node[1].can_msg_obj_config);
    //rcv msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_can.multi_can_node[1].can_msg_obj_config,
                                      &g_drv_multi_can.multi_can_node[1].can_node);

    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.msgObjId = RECV_MESSAGE_OBJECT_ID_0;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.messageId = 0;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.frame = IfxMultican_Frame_receive;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.rxInterrupt.enabled = TRUE;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.txInterrupt.srcId = IfxMultican_SrcId_2;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_3;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.acceptanceMask = 0;
    g_drv_multi_can.multi_can_node[1].can_msg_obj_config.control.matchingId = TRUE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_can.multi_can_node[1].can_rcv_msgobj,
                                &g_drv_multi_can.multi_can_node[1].can_msg_obj_config);
    Ifx_print("init_multi_can_node 1 done.\r\n");
}

void multi_can_node2_init(void)
{
    IfxMultican_Can_Node_initConfig(&g_drv_multi_can.multi_can_node[2].can_node_config, &g_drv_multi_can.can);

    g_drv_multi_can.multi_can_node[2].can_node_config.nodeId = IfxMultican_NodeId_2;

    g_drv_multi_can.multi_can_node[2].can_node_config.rxPin = &IfxMultican_RXD2A_P15_1_IN;
    g_drv_multi_can.multi_can_node[2].can_node_config.rxPinMode = IfxPort_InputMode_pullUp;
    g_drv_multi_can.multi_can_node[2].can_node_config.txPin = &IfxMultican_TXD2_P15_0_OUT;
    g_drv_multi_can.multi_can_node[2].can_node_config.txPinMode = IfxPort_OutputMode_pushPull;

    //node 0
    IfxMultican_Can_Node_init(&g_drv_multi_can.multi_can_node[2].can_node, &g_drv_multi_can.multi_can_node[2].can_node_config);

    //send msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_can.multi_can_node[2].can_msg_obj_config,
                                      &g_drv_multi_can.multi_can_node[2].can_node);

    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.msgObjId = SEND_MESSAGE_OBJECT_ID_0;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.messageId = CAN_MESSAGE_ID;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.frame = IfxMultican_Frame_transmit;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.txInterrupt.enabled = TRUE;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.txInterrupt.srcId = IfxMultican_SrcId_4;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_5;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.control.matchingId = TRUE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_can.multi_can_node[2].can_send_msgobj,
                                &g_drv_multi_can.multi_can_node[2].can_msg_obj_config);
    //rcv msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_can.multi_can_node[2].can_msg_obj_config,
                                      &g_drv_multi_can.multi_can_node[2].can_node);

    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.msgObjId = RECV_MESSAGE_OBJECT_ID_0;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.messageId = 0;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.frame = IfxMultican_Frame_receive;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.rxInterrupt.enabled = TRUE;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.txInterrupt.srcId = IfxMultican_SrcId_4;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_5;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.acceptanceMask = 0;
    g_drv_multi_can.multi_can_node[2].can_msg_obj_config.control.matchingId = TRUE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_can.multi_can_node[2].can_rcv_msgobj,
                                &g_drv_multi_can.multi_can_node[2].can_msg_obj_config);
    Ifx_print("init_multi_can_node 2 done.\r\n");
}

void multi_can_node3_init(void)
{
    IfxMultican_Can_Node_initConfig(&g_drv_multi_can.multi_can_node[3].can_node_config, &g_drv_multi_can.can);

    g_drv_multi_can.multi_can_node[3].can_node_config.nodeId = IfxMultican_NodeId_3;

    g_drv_multi_can.multi_can_node[3].can_node_config.rxPin = &IfxMultican_RXD3E_P20_9_IN;
    g_drv_multi_can.multi_can_node[3].can_node_config.rxPinMode = IfxPort_InputMode_pullUp;
    g_drv_multi_can.multi_can_node[3].can_node_config.txPin = &IfxMultican_TXD3_P20_10_OUT;
    g_drv_multi_can.multi_can_node[3].can_node_config.txPinMode = IfxPort_OutputMode_pushPull;

    //node 0
    IfxMultican_Can_Node_init(&g_drv_multi_can.multi_can_node[3].can_node, &g_drv_multi_can.multi_can_node[3].can_node_config);

    //send msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_can.multi_can_node[3].can_msg_obj_config,
                                      &g_drv_multi_can.multi_can_node[3].can_node);

    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.msgObjId = SEND_MESSAGE_OBJECT_ID_0;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.messageId = CAN_MESSAGE_ID;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.frame = IfxMultican_Frame_transmit;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.txInterrupt.enabled = TRUE;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.txInterrupt.srcId = IfxMultican_SrcId_6;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_7;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.control.matchingId = TRUE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_can.multi_can_node[3].can_send_msgobj,
                                &g_drv_multi_can.multi_can_node[3].can_msg_obj_config);
    //rcv msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_can.multi_can_node[3].can_msg_obj_config,
                                      &g_drv_multi_can.multi_can_node[3].can_node);

    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.msgObjId = RECV_MESSAGE_OBJECT_ID_0;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.messageId = 0;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.frame = IfxMultican_Frame_receive;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.rxInterrupt.enabled = TRUE;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.txInterrupt.srcId = IfxMultican_SrcId_6;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_7;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.acceptanceMask = 0;
    g_drv_multi_can.multi_can_node[3].can_msg_obj_config.control.matchingId = TRUE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_can.multi_can_node[3].can_rcv_msgobj,
                                &g_drv_multi_can.multi_can_node[3].can_msg_obj_config);
    Ifx_print("init_multi_can_node 3 done.\r\n");
}

void multi_can_module_init()
{
    IfxMultican_Can_initModuleConfig(&g_drv_multi_can.can_config, &MODULE_CAN);

    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_0].priority = ISR_PRIORITY_CAN_TX_0;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_0].typeOfService = IfxSrc_Tos_cpu1;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_1].priority = ISR_PRIORITY_CAN_RX_0;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_1].typeOfService = IfxSrc_Tos_cpu1;

    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_2].priority = ISR_PRIORITY_CAN_TX_1;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_2].typeOfService = IfxSrc_Tos_cpu1;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_3].priority = ISR_PRIORITY_CAN_RX_1;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_3].typeOfService = IfxSrc_Tos_cpu1;

    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_4].priority = ISR_PRIORITY_CAN_TX_2;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_4].typeOfService = IfxSrc_Tos_cpu1;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_5].priority = ISR_PRIORITY_CAN_RX_2;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_5].typeOfService = IfxSrc_Tos_cpu1;

    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_6].priority = ISR_PRIORITY_CAN_TX_3;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_6].typeOfService = IfxSrc_Tos_cpu1;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_7].priority = ISR_PRIORITY_CAN_RX_3;
    g_drv_multi_can.can_config.nodePointer[IfxMultican_SrcId_7].typeOfService = IfxSrc_Tos_cpu1;

    IfxMultican_Can_initModule(&g_drv_multi_can.can, &g_drv_multi_can.can_config);
}

void multi_can_send_msg(int node_index)
{
    /* Define the content of the data to be transmitted */
    uint32 dataLow = 0xC0CAC01A;
    uint32 dataHigh = 0xBA5EBA11;

    /* Invalidation of the RX message */

    /* Initialization of the TX message */
    IfxMultican_Message_init(&g_drv_multi_can.multi_can_node[node_index].can_tx_msg,
                             g_drv_multi_can.multi_can_node[node_index].can_msg_obj_config.messageId,
                             dataLow,
                             dataHigh,
                             g_drv_multi_can.multi_can_node[node_index].can_msg_obj_config.control.messageLen);

    /* Send the CAN message with the previously defined TX message content */
    while (IfxMultican_Status_notSentBusy ==
           IfxMultican_Can_MsgObj_sendMessage(&g_drv_multi_can.multi_can_node[node_index].can_send_msgobj,
                                              &g_drv_multi_can.multi_can_node[node_index].can_tx_msg))
    {
    }
}
