/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"

#include "Src/Std/IfxSrc.h"

#include "ethernetif.h"

#include <string.h>

#include "Pub.h"
#include "cc.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

#if ETH_PAD_SIZE
#define PBUF_DROP_PAD(p) pbuf_header(p, -ETH_PAD_SIZE)
#define PBUF_CLAIM_PAD(p) pbuf_header(p, ETH_PAD_SIZE)
#else
#define PBUF_DROP_PAD(p)
#define PBUF_CLAIM_PAD(p)
#endif

/*Maximum retry iterations for phy auto-negotiation*/

#define IFX_LWIP_ZERO_COPY_BUFFERS (4)

#define ETH_LWIP_PHY_MAX_RETRIES 0xfffffU

struct ethernetif_tc2x g_ethernetif_tc2x;

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
  IfxEth *eth = &g_drv_eth.eth;
  int i = 0;
  Ifx_print("low_level_init mac is \r\n");
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;
  for (i = 0; i < netif->hwaddr_len; i++)
  {
    Ifx_print("%02x ", netif->hwaddr[i]);
  }
  Ifx_print(" \r\n");

  init_eth_module(netif->hwaddr);

#if LWIP_USE_HW_CHECKSUM_ENGINE

  IfxEth_setupChecksumEngine(eth, IfxEth_ChecksumMode_tcpUdpIcmpFull);
#endif

  IfxEth_startTransmitter(eth);
  IfxEth_startReceiver(eth);
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  IfxEth *eth = &g_drv_eth.eth;
  struct pbuf *q;
  LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE, ("low_level_output in \r\n"));
  u16_t length = p->tot_len;
  LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE, ("low_level_output %p\n", (void *)p));

  PBUF_DROP_PAD(p);

  //initiate transfer();
  LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE, ("low_level_output in \r\n"));

  //if ((p->type == PBUF_REF) || (p->type == PBUF_ROM))
  {

    u8_t *tbuf = (u8_t *)IfxEth_waitTransmitBuffer(eth);
    u16_t l = 0;

    for (q = p; q != NULL; q = q->next)
    {
      /* Send the data from the pbuf to the interface, one pbuf at a
             * time. The size of the data in each pbuf is kept in the ->len
             * variable. */
      memcpy(&tbuf[l], q->payload, q->len);
      l = l + q->len;
      LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE, ("low_level_output: data=%#x, %d\n", q->payload, q->len));
      LWIP_ASSERT("low_level_output: length overflow the buffer\n", (l < IFXETH_RTX_BUFFER_SIZE));
    }

    IfxEth_sendTransmitBuffer(eth, l);
  }

  LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE, ("low_level_output: signal length: %d\n", length));

  PBUF_CLAIM_PAD(p);

  //LINK_STATS_INC(link.xmit);

  LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE, ("low_level_output: return OK\n"));

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *low_level_input(struct netif *netif)
{
  IfxEth *eth = &g_drv_eth.eth;
  struct pbuf *p = NULL, *q = NULL;
  u16_t len;

  LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE, ("low_level_input in \r\n"));

  len = IfxEth_getRxDataLength(eth);

  if (len == 0)
  { /* no reception */
    p = (struct pbuf *)0;
  }
  else
  {
    len = len + (ETH_PAD_SIZE ? ETH_PAD_SIZE : 0); /* allow room for Ethernet padding */

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

    if (p != NULL)
    {
      PBUF_DROP_PAD(p);
      u8_t *src = IfxEth_getReceiveBuffer(eth);

      /* We iterate over the pbuf chain until we have read the entire
             * packet into the pbuf. */
      for (q = p; q != NULL; q = q->next)
      {
        /* Read enough bytes to fill this pbuf in the chain. The
                 * available data in the pbuf is given by the q->len
                 * variable.
                 * This does not necessarily have to be a memcpy, you can also preallocate
                 * pbufs for a DMA-enabled MAC and after receiving truncate it to the
                 * actually received size. In this case, ensure the tot_len member of the
                 * pbuf is the sum of the chained pbuf len members.
                 */
        //read data into(q->payload, q->len);
        memcpy(q->payload, src, q->len);
        src = &src[q->len];

        LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE, ("low_level_input: payload=0x%x, len=%d\n", q->payload, q->len));
      }

      //acknowledge that packet has been read();
      IfxEth_freeReceiveBuffer(eth);

      PBUF_CLAIM_PAD(p);
      LINK_STATS_INC(link.recv);
    }

    else
    {
      //TODO: drop packet();
      __debug();
      LINK_STATS_INC(link.memerr);
      LINK_STATS_INC(link.drop);
    }
  }

  return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */

void ethernetif_input(struct netif *netif)
{
  //IfxEth *eth = netif->state;
  err_t err = ERR_OK;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: p == NULL!\n"));
    err = ERR_TIMEOUT;
  }
  else
  {
    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = p->payload;
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: %x \r\n", htons(ethhdr->type)));
    switch (htons(ethhdr->type))
    {
    /* IP or ARP packet? */
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
#if PPPOE_SUPPORT
    /* PPPoE packet? */
    case ETHTYPE_PPPOEDISC:
    case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */

      /* full packet send to tcpip_thread to process */
      if (netif->input(p, netif) != ERR_OK)
      {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
        pbuf_free(p);
      }

      break;

    default:
      LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: type unknown\n"));
      pbuf_free(p);
      break;
    }
  }
}

static void ethernetif_link_callback(struct netif *netif)
{
  static int link_flag = 0;
  int link_state = 0;

  link_state = IfxEth_isLinkActive(&g_drv_eth.eth);
  if (link_state)
  {
    if (link_flag == 0)
    {
      Ifx_print("ethernetif_link_callback will set up netif %p\r\n", netif);
      netif_set_up(netif);
      netif_set_link_up(netif);
    }
    link_flag = 1;
  }
  else
  {
    if (link_flag == 1)
    {
      Ifx_print("ethernetif_link_callback will set down netif %p\r\n", netif);
      netif_set_down(netif);
      netif_set_link_down(netif);
    }
    link_flag = 0;
  }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;

  g_ethernetif_tc2x.eth = &g_drv_eth.eth;
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */

  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  netif->link_callback = ethernetif_link_callback;
  /* initialize the hardware */
  low_level_init(netif);

#if LWIP_IPV6 && LWIP_IPV6_MLD
  /*
   * For hardware/netifs that implement MAC filtering.
   * All-nodes link-local is handled by default, so we must let the hardware know
   * to allow multicast packets in.
   * Should set mld_mac_filter previously. */
  if (netif->mld_mac_filter != NULL)
  {
    ip6_addr_t ip6_allnodes_ll;
    ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
    netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
  }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

  /*  Registers the default network interface.*/
  netif_set_default(netif);

  //netif_set_link_up(netif);

  return ERR_OK;
}

void ethernetif_recv(struct netif *netif)
{

  if (NULL == g_eth_swamphore)
  {
    Ifx_print("g_eth_swamphore Failed...\r\n");
  }
  for (;;)
  {
    //Ifx_print("ethernetif_recv g_eth_swamphore\r\n");
    xSemaphoreTake(g_eth_swamphore, portMAX_DELAY);
    ethernetif_input(netif);
  }
}

void ethernetif_poll(struct netif *netif)
{
  ethernetif_input(netif);
}

#if 0
int Ifx_Lwip_printf(const char *format, ...)
{
  /* WNC do not have console port */
  int result = 0;
  if (!(g_drv_asc_lin_uart.asc_sandard_interface.txDisabled))
  {
    char message[STDIF_DPIPE_MAX_PRINT_SIZE + 1];
    Ifx_SizeT count;
    va_list args;
    va_start(args, format);
    vsprintf((char *)message, format, args);
    va_end(args);
    count = (Ifx_SizeT)strlen(message);
    IFX_ASSERT(IFX_VERBOSE_LEVEL_ERROR, count < STDIF_DPIPE_MAX_PRINT_SIZE);
    //return
    IfxStdIf_DPipe_write(&g_drv_asc_lin_uart.asc_sandard_interface, (void *)message, &count, TIME_INFINITE);
  }
  else
  {
    //return TRUE;
  }
  return result;
}
#else
int Ifx_Lwip_printf(const char *format, ...)
{
  return 0;
}
#endif
