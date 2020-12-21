#include "hDrv.h"
#include "Pub.h"

int itimes_can4_rcv = 0;
int itimes_can5_rcv = 0;
int stat_4 = 0, stat_5 = 0;

#define CAN_MESSAGE_ID 0x777 /* Message ID that will be used in arbitration phase */

#define INVALID_DATA_VALUE (uint32)0xDEADBEEF /* Used to invalidate RX message data content        */
#define INVALID_ID_VALUE (uint32)0xFFFFFFFF   /* Used to invalidate RX message ID value            */

#define SEND_MESSAGE_OBJECT_ID_4 (IfxMultican_MsgObjId)8  /* Source message object ID                          */
#define RECV_MESSAGE_OBJECT_ID_4 (IfxMultican_MsgObjId)9  /* Destination message object ID                     */
#define SEND_MESSAGE_OBJECT_ID_5 (IfxMultican_MsgObjId)10 /* Source message object ID                          */
#define RECV_MESSAGE_OBJECT_ID_5 (IfxMultican_MsgObjId)11 /* Destination message object ID                     */
/**canr_1**/

#define ISR_PRIORITY_CAN_RX_4 18 /* Define the CAN RX interrupt priority              */
#define ISR_PRIORITY_CAN_RX_5 19 /* Define the CAN RX interrupt priority              */

#define ISR_PRIORITY_CAN_ALERT_4 20 /* Define the CAN TX interrupt priority              */
#define ISR_PRIORITY_CAN_ALERT_5 21 /* Define the CAN TX interrupt priority              */


struct drv_multi_canr g_drv_multi_canr;

IFX_INTERRUPT(canIsrRxHandler_4, 0, ISR_PRIORITY_CAN_RX_4);
IFX_INTERRUPT(canIsrRxHandler_5, 0, ISR_PRIORITY_CAN_RX_5);

IFX_INTERRUPT(canIsrAlertHandler_4, 0, ISR_PRIORITY_CAN_ALERT_4);
IFX_INTERRUPT(canIsrAlertHandler_5, 0, ISR_PRIORITY_CAN_ALERT_5);

void canIsrAlertHandler_4(void)
{
    unsigned int status = 0;
    BaseType_t xHigherPriorityTaskWoken;

    IfxMultican_Can_Node *node = &g_drv_multi_canr.multi_can_node[0].can_node;
    Ifx_CAN_N *hwNode = IfxMultican_Node_getPointer(node->mcan, node->nodeId);
    status = hwNode->SR.U;
    stat_4 = status;
    xHigherPriorityTaskWoken = pdFALSE;

    if ((status & 0x00000007) == 0x05)
    {
        IfxMultican_Node_setAlertInterrupt(hwNode, 0);
        xSemaphoreGiveFromISR(g_can_restart_phore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        restart_num = 4;
    }
}

void canIsrAlertHandler_5(void)
{
    unsigned int status = 0;
    BaseType_t xHigherPriorityTaskWoken;

    IfxMultican_Can_Node *node = &g_drv_multi_canr.multi_can_node[1].can_node;
    Ifx_CAN_N *hwNode = IfxMultican_Node_getPointer(node->mcan, node->nodeId);
    status = hwNode->SR.U;
    stat_5 = status;
    xHigherPriorityTaskWoken = pdFALSE;

    if ((status & 0x00000007) == 0x05)
    {
        IfxMultican_Node_setAlertInterrupt(hwNode, 0);
        xSemaphoreGiveFromISR(g_can_restart_phore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        restart_num = 5;
    }
}

void canIsrRxHandler_4(void)
{
    IfxMultican_Status readStatus;
    IfxMultican_Can_MsgObj *msgObj = &g_drv_multi_canr.multi_can_node[0].can_rcv_msgobj;
    Ifx_CAN *mcanSFR = msgObj->node->mcan;

    Ifx_CAN_MO *hwObj = IfxMultican_MsgObj_getPointer(mcanSFR, msgObj->msgObjId);
    IfxMultican_MsgObj_setReceiveInterrupt(hwObj, 0);
    itimes_can4_rcv++;
    readStatus = IfxMultican_Can_MsgObj_readMessage(&g_drv_multi_canr.multi_can_node[0].can_rcv_msgobj,
                                                    &g_drv_multi_canr.multi_can_node[0].can_rx_msg);
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
    IfxMultican_MsgObj_setReceiveInterrupt(hwObj, 1);
}

void canIsrRxHandler_5(void)
{
    IfxMultican_Status readStatus;
    IfxMultican_Can_MsgObj *msgObj = &g_drv_multi_canr.multi_can_node[1].can_rcv_msgobj;
    Ifx_CAN *mcanSFR = msgObj->node->mcan;

    Ifx_CAN_MO *hwObj = IfxMultican_MsgObj_getPointer(mcanSFR, msgObj->msgObjId);
    IfxMultican_MsgObj_setReceiveInterrupt(hwObj, 0);
    itimes_can5_rcv++;
    readStatus = IfxMultican_Can_MsgObj_readMessage(&g_drv_multi_canr.multi_can_node[1].can_rcv_msgobj,
                                                    &g_drv_multi_canr.multi_can_node[1].can_rx_msg);
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
    IfxMultican_MsgObj_setReceiveInterrupt(hwObj, 1);
}

void multi_can_node4_init(void)
{
    IfxMultican_Can_Node_initConfig(&g_drv_multi_canr.multi_can_node[0].can_node_config, &g_drv_multi_canr.can);

    g_drv_multi_canr.multi_can_node[0].can_node_config.nodeId = IfxMultican_NodeId_0;

    g_drv_multi_canr.multi_can_node[0].can_node_config.rxPin = &IfxCanr_RXD0C_P34_2_IN;
    g_drv_multi_canr.multi_can_node[0].can_node_config.rxPinMode = IfxPort_InputMode_pullUp;
    g_drv_multi_canr.multi_can_node[0].can_node_config.txPin = &IfxCanr_TXD0_P34_1_OUT;
    g_drv_multi_canr.multi_can_node[0].can_node_config.txPinMode = IfxPort_OutputMode_pushPull;

    g_drv_multi_canr.multi_can_node[0].can_node_config.alertInterrupt.enabled = TRUE;
    g_drv_multi_canr.multi_can_node[0].can_node_config.alertInterrupt.srcId = IfxMultican_SrcId_2;

    //node 0
    IfxMultican_Can_Node_init(&g_drv_multi_canr.multi_can_node[0].can_node, &g_drv_multi_canr.multi_can_node[0].can_node_config);

    //send msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_canr.multi_can_node[0].can_msg_obj_config,
                                      &g_drv_multi_canr.multi_can_node[0].can_node);

    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.msgObjId = SEND_MESSAGE_OBJECT_ID_4;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.messageId = CAN_MESSAGE_ID;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.frame = IfxMultican_Frame_transmit;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.control.matchingId = FALSE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_canr.multi_can_node[0].can_send_msgobj,
                                &g_drv_multi_canr.multi_can_node[0].can_msg_obj_config);
    //rcv msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_canr.multi_can_node[0].can_msg_obj_config,
                                      &g_drv_multi_canr.multi_can_node[0].can_node);

    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.msgObjId = RECV_MESSAGE_OBJECT_ID_4;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.messageId = 4;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.frame = IfxMultican_Frame_receive;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.rxInterrupt.enabled = TRUE;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_0;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.acceptanceMask = 0;
    g_drv_multi_canr.multi_can_node[0].can_msg_obj_config.control.matchingId = FALSE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_canr.multi_can_node[0].can_rcv_msgobj,
                                &g_drv_multi_canr.multi_can_node[0].can_msg_obj_config);
    Ifx_print1("init_multi_can_node 4 done.\r\n");
}

void multi_can_node5_init(void)
{
    IfxMultican_Can_Node_initConfig(&g_drv_multi_canr.multi_can_node[1].can_node_config, &g_drv_multi_canr.can);

    g_drv_multi_canr.multi_can_node[1].can_node_config.nodeId = IfxMultican_NodeId_1;

    g_drv_multi_canr.multi_can_node[1].can_node_config.rxPin = &IfxCanr_RXD1A_P00_3_IN;
    g_drv_multi_canr.multi_can_node[1].can_node_config.rxPinMode = IfxPort_InputMode_pullUp;
    g_drv_multi_canr.multi_can_node[1].can_node_config.txPin = &IfxCanr_TXD1_P00_2_OUT;
    g_drv_multi_canr.multi_can_node[1].can_node_config.txPinMode = IfxPort_OutputMode_pushPull;

    g_drv_multi_canr.multi_can_node[1].can_node_config.alertInterrupt.enabled = TRUE;
    g_drv_multi_canr.multi_can_node[1].can_node_config.alertInterrupt.srcId = IfxMultican_SrcId_3;

    //node 0
    IfxMultican_Can_Node_init(&g_drv_multi_canr.multi_can_node[1].can_node, &g_drv_multi_canr.multi_can_node[1].can_node_config);

    //send msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_canr.multi_can_node[1].can_msg_obj_config,
                                      &g_drv_multi_canr.multi_can_node[1].can_node);

    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.msgObjId = SEND_MESSAGE_OBJECT_ID_5;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.messageId = CAN_MESSAGE_ID;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.frame = IfxMultican_Frame_transmit;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.control.matchingId = FALSE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_canr.multi_can_node[1].can_send_msgobj,
                                &g_drv_multi_canr.multi_can_node[1].can_msg_obj_config);
    //rcv msg obj
    IfxMultican_Can_MsgObj_initConfig(&g_drv_multi_canr.multi_can_node[1].can_msg_obj_config,
                                      &g_drv_multi_canr.multi_can_node[1].can_node);

    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.msgObjId = RECV_MESSAGE_OBJECT_ID_5;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.messageId = 5;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.frame = IfxMultican_Frame_receive;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.rxInterrupt.enabled = TRUE;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.rxInterrupt.srcId = IfxMultican_SrcId_1;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.acceptanceMask = 0;
    g_drv_multi_canr.multi_can_node[1].can_msg_obj_config.control.matchingId = FALSE;
    IfxMultican_Can_MsgObj_init(&g_drv_multi_canr.multi_can_node[1].can_rcv_msgobj,
                                &g_drv_multi_canr.multi_can_node[1].can_msg_obj_config);
    Ifx_print1("init_multi_can_node 5 done.\r\n");
}

void multi_canr_module_init()
{
    IfxMultican_Can_initModuleConfig(&g_drv_multi_canr.can_config, &MODULE_CANR);

    g_drv_multi_canr.can_config.nodePointer[IfxMultican_SrcId_0].priority = ISR_PRIORITY_CAN_RX_4;
    g_drv_multi_canr.can_config.nodePointer[IfxMultican_SrcId_0].typeOfService = IfxSrc_Tos_cpu1;
    g_drv_multi_canr.can_config.nodePointer[IfxMultican_SrcId_1].priority = ISR_PRIORITY_CAN_RX_5;
    g_drv_multi_canr.can_config.nodePointer[IfxMultican_SrcId_1].typeOfService = IfxSrc_Tos_cpu1;

    g_drv_multi_canr.can_config.nodePointer[IfxMultican_SrcId_2].priority = ISR_PRIORITY_CAN_ALERT_4;
    g_drv_multi_canr.can_config.nodePointer[IfxMultican_SrcId_2].typeOfService = IfxSrc_Tos_cpu1;
    g_drv_multi_canr.can_config.nodePointer[IfxMultican_SrcId_3].priority = ISR_PRIORITY_CAN_ALERT_5;
    g_drv_multi_canr.can_config.nodePointer[IfxMultican_SrcId_3].typeOfService = IfxSrc_Tos_cpu1;


    IfxMultican_Can_initModule(&g_drv_multi_canr.can, &g_drv_multi_canr.can_config);
}

void multi_canr_send_msg(int node_index)
{
    /* Define the content of the data to be transmitted */
    uint32 dataLow = 0xC0CAC01A;
    uint32 dataHigh = 0xBA5EBA11;

    /* Invalidation of the RX message */

    /* Initialization of the TX message */
    IfxMultican_Message_init(&g_drv_multi_canr.multi_can_node[node_index].can_tx_msg,
                             g_drv_multi_canr.multi_can_node[node_index].can_msg_obj_config.messageId,
                             dataLow,
                             dataHigh,
                             g_drv_multi_canr.multi_can_node[node_index].can_msg_obj_config.control.messageLen);

    /* Send the CAN message with the previously defined TX message content */
    while (IfxMultican_Status_notSentBusy ==
           IfxMultican_Can_MsgObj_sendMessage(&g_drv_multi_canr.multi_can_node[node_index].can_send_msgobj,
                                              &g_drv_multi_canr.multi_can_node[node_index].can_tx_msg))
    {
    }
}
