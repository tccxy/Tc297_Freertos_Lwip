#include "Pub.h"
#include "lwip_demo.h"

/****Ŀǰ��������ĺ���ΪCPU1
 **��Ҫ�޸�hEth.c  **�ļ�
*****/

#define ISR_PRIORITY_ETH_IRQ_BARE 4
static struct netif xnetif_bare =
    {
        /* set MAC hardware address length */
        .hwaddr_len = (u8_t)ETH_HWADDR_LEN,

        /* set MAC hardware address */
        .hwaddr = {0x00, 0x20, 0x30, 0x40, 0x50, 0x60},

        /* maximum transfer unit */
        .mtu = 1500U,
};

static void net_init(void)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    Ifx_print1("net_init no dhcp\r\n");
    IP4_ADDR(&ipaddr, 192, 168, 5, 123);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 168, 5, 1);

    /* Create tcp_ip stack thread */
    Ifx_print1("net_init \r\n");
    lwip_init();
    //tcpip_init(NULL, NULL);
    Ifx_print1("net_init tcpip_init down \r\n");

    /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
  struct ip_addr *netmask, struct ip_addr *gw,
  void *state, err_t (* init)(struct netif *netif),
  err_t (* input)(struct pbuf *p, struct netif *netif))

  Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
    netif_add(&xnetif_bare, &ipaddr, &netmask, &gw, (void *)0, &ethernetif_init, ethernet_input);
    Ifx_print1("net_init netif_add down \r\n");

    netif_set_link_up(&xnetif_bare);
    netif_set_up(&xnetif_bare);

    //IfxSrc_init(&SRC_ETH_ETH0_SR, IfxSrc_Tos_cpu0, ISR_PRIORITY_ETH_IRQ);
    //IfxSrc_enable(&SRC_ETH_ETH0_SR);
    //Ifx_print("net_init netif_set_default down \r\n");
    Ifx_print1("net_init down \r\n");
}

IFX_INTERRUPT(ETH_IRQHandler_bare, 0, ISR_PRIORITY_ETH_IRQ_BARE);
void ETH_IRQHandler_bare(void)
{
    /* Disable further ETH RX interrupts */
    if (IfxEth_isRxInterrupt(&g_drv_eth.eth))
    {
        //led_107_on();
        IfxPort_setPinState(LED1, IfxPort_State_toggled);
        IfxEth_clearRxInterrupt(&g_drv_eth.eth);
        ethernetif_poll(&xnetif_bare);
    }
    if (IfxEth_isTxInterrupt(&g_drv_eth.eth))
    {
        led_108_on();
        IfxEth_clearTxInterrupt(&g_drv_eth.eth);
    }
    if (IfxEth_isTuInterrupt(&g_drv_eth.eth))
    {
        led_109_on();
        IfxEth_clearTuInterrupt(&g_drv_eth.eth);
    }
    IfxSrc_clearRequest(&SRC_ETH_ETH0_SR);
}

void recv_callback_bare(void *arg, struct udp_pcb *upcb, struct pbuf *pkt_buf,
                   const ip_addr_t *addr, u16_t port)
{
    char data[100] = {0};
    Ifx_print1("recv_callback_bare in \r\n");
    /* process new connection request */
    memcpy(data, pkt_buf->payload, pkt_buf->len);
    Ifx_print1("data is %s \r\n", data);
    pbuf_free(pkt_buf);
}

void test_eth_bare(void)
{
    struct udp_pcb *udp;
    err_t err;
    ip_addr_t dest_addr;
    struct pbuf *ppkt_buf;
    char data_test[10] = {1};
    int n = 100000000, i = 0;

    Ifx_print1("net_init no dhcp\r\n");
    net_init();

    IP4_ADDR(&dest_addr, 192, 168, 5, 196);
    udp = udp_new();

    udp_bind(udp, IP_ADDR_ANY, 0);

    err = udp_connect(udp, &dest_addr, UDP_SERVER_PORT);
    if (err != ERR_OK)
    {
        Ifx_print1("udp_connect error.%x \r\n", err);
        while (1)
        {
        }
    }
    udp_recv(udp, recv_callback_bare, NULL);
    for (;;)
    {
        i++;
        while (n--)
            ;
        n = 3000000;
        ppkt_buf = pbuf_alloc(PBUF_TRANSPORT, sizeof(data_test), PBUF_POOL);
        if (ppkt_buf != NULL)
        {

            Ifx_print1("send -- %d- msg %p\r\n", i, ppkt_buf);
            memcpy(ppkt_buf->payload, data_test, sizeof(data_test));
            udp_sendto(udp, ppkt_buf, &dest_addr, UDP_SERVER_PORT);
            //System_Periodic_Handle();
            pbuf_free(ppkt_buf);
        }
    }
    udp_remove(udp);
}
