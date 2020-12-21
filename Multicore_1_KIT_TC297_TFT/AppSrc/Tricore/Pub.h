#ifndef _PUB_H
#define _PUB_H

#include "Bsp.h"
#include "Cpu/Std/Ifx_Types.h"
#include "IfxScuWdt.h"
#include "Ifx_Types.h"
#include "IfxMultican.h"
#include "string.h"

#include "Ifx_Types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hDrv.h"
#include "semphr.h"

#include <netif.h>
#include <tcpip.h>
#include <etharp.h>
#include <ethernet.h>
#include <ethernetif.h>
#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "sockets.h"
#include "udp.h"
#include "ip4_addr.h"

extern SemaphoreHandle_t g_eth_swamphore;
extern SemaphoreHandle_t g_can_restart_phore;

extern struct drv_multi_can g_drv_multi_can;
extern int itimes_can0_rcv;
extern int itimes_can1_rcv;
extern int itimes_can2_rcv;
extern int itimes_can3_rcv;
extern int itimes_can4_rcv;
extern int itimes_can5_rcv;
extern int restart_num;
extern int stat_0;
extern int stat_1;
extern int stat_2;
extern int stat_3;
extern int stat_4;
extern int stat_5;
#endif
