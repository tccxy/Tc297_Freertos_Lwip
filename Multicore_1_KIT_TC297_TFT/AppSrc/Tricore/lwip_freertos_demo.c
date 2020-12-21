#include "Pub.h"
#include "lwip_demo.h"

static struct netif xnetif_fos =
    {
        /* set MAC hardware address length */
        .hwaddr_len = (u8_t)ETH_HWADDR_LEN,

        /* set MAC hardware address */
        .hwaddr = {0x00, 0x20, 0x30, 0x40, 0x50, 0x60},

        /* maximum transfer unit */
        .mtu = 1500U,
};

#define ISR_PRIORITY_ETH_IRQ_FOS 6

void eth_link_monitor(void *pvParameters)
{
    while (1)
    {
        xnetif_fos.link_callback(&xnetif_fos);
        vTaskDelay(1000);
    }
}

static void net_init(void)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    IP4_ADDR(&ipaddr, 192, 168, 5, 123);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 168, 5, 1);

    /* Create tcp_ip stack thread */
    lwip_init();
    netif_add(&xnetif_fos, &ipaddr, &netmask, &gw, (void *)0, &ethernetif_init, ethernet_input);

    /**/
    Ifx_print("eth_link_monitor will run %p \r\n", &xnetif_fos);

    xTaskCreate(eth_link_monitor, "eth_link_monitor", 1024, NULL, 4, NULL);
    Ifx_print("net_init down \r\n");
}

static void net_init_socket(void)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    IP4_ADDR(&ipaddr, 192, 168, 5, 123);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 168, 5, 1);

    /* Create tcp_ip stack thread */
    tcpip_init(NULL, NULL);
    netif_add(&xnetif_fos, &ipaddr, &netmask, &gw, (void *)0, &ethernetif_init, tcpip_input);

    //netif_set_up(&xnetif_fos);
    Ifx_print("eth_link_monitor will run %p \r\n", &xnetif_fos);
    xTaskCreate(eth_link_monitor, "eth_link_monitor", 1024, NULL, 4, NULL);
    Ifx_print("net_init_socket down \r\n");
}

void eth_init_fos(void *pvParameters)
{
    //net_init();//when test_eth_fos use this
    net_init_socket(); // when test_eth_socket* use this
    ethernetif_recv(&xnetif_fos);
}

int irq_rx = 0, irq_all = 0, irq_tx = 0, irq_tu = 0, irq_eri = 0, irq_nis = 0;
unsigned int eth_flag = 0;
IFX_INTERRUPT(ETH_IRQHandler_fos, 0, ISR_PRIORITY_ETH_IRQ_FOS);
void ETH_IRQHandler_fos(void)
{
    BaseType_t xHigherPriorityTaskWoken;

    ETH_INTERRUPT_ENABLE.U = 0; /* disable tx & rx interrupts */
    xHigherPriorityTaskWoken = pdFALSE;
    irq_all++;
    if (g_eth_swamphore != NULL)
    {
        xSemaphoreGiveFromISR(g_eth_swamphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    /* Disable further ETH RX interrupts */
    if (IfxEth_isRxInterrupt(&g_drv_eth.eth))
    {
        irq_rx++;
        //led_107_on();
        //IfxPort_setPinState(LED1, IfxPort_State_toggled);
        IfxEth_clearRxInterrupt(&g_drv_eth.eth);
    }
    if (IfxEth_isTxInterrupt(&g_drv_eth.eth))
    {
        irq_tx++;
        //led_108_on();
        IfxEth_clearTxInterrupt(&g_drv_eth.eth);
    }

    if (IfxEth_isTuInterrupt(&g_drv_eth.eth))
    {
        irq_tu++;
        IfxEth_clearTuInterrupt(&g_drv_eth.eth);
    }

    if (IfxEth_isEriInterrupt(&g_drv_eth.eth))
    {
        irq_eri++;
        IfxEth_clearEriInterrupt(&g_drv_eth.eth);
    }

    if (IfxEth_isNisInterrupt(&g_drv_eth.eth))
    {
        irq_nis++;
        IfxEth_clearNisInterrupt(&g_drv_eth.eth);
    }
    eth_flag = IfxEth_readAllEthStatus(&g_drv_eth.eth);

    IfxSrc_clearRequest(&SRC_ETH_ETH0_SR);
    ETH_INTERRUPT_ENABLE.U = 0x00010041; /* enable tx & rx interrupts */
}

void recv_callback_fos(void *arg, struct udp_pcb *upcb, struct pbuf *pkt_buf,
                       const ip_addr_t *addr, u16_t port)
{
    char data[100] = {0};
    Ifx_print("recv_callback_fos in \r\n");
    /* process new connection request */
    memcpy(data, pkt_buf->payload, pkt_buf->len);
    Ifx_print("data is %s \r\n", data);
    pbuf_free(pkt_buf);
}

void test_eth_fos(void *pvParameters)
{
    struct udp_pcb *udp;
    err_t err;
    ip_addr_t dest_addr;
    struct pbuf *ppkt_buf;
    char data_test[10] = {1};
    int i = 0;

    Ifx_print("test_eth_fos init\r\n");
    IP4_ADDR(&dest_addr, 192, 168, 5, 196);
    udp = udp_new();

    udp_bind(udp, IP_ADDR_ANY, 0);

    err = udp_connect(udp, &dest_addr, UDP_SERVER_PORT);
    if (err != ERR_OK)
    {
        Ifx_print("udp_connect error.%x \r\n", err);
        while (1)
        {
        }
    }
    udp_recv(udp, recv_callback_fos, NULL);
    Ifx_print("test_eth_fos down\r\n");

    while (1)
    {
        ppkt_buf = pbuf_alloc(PBUF_TRANSPORT, sizeof(data_test), PBUF_POOL);
        if (ppkt_buf != NULL)
        {

            Ifx_print("send -- %d- msg %p\r\n", i++, ppkt_buf);
            memcpy(ppkt_buf->payload, data_test, sizeof(data_test));
            udp_sendto(udp, ppkt_buf, &dest_addr, UDP_SERVER_PORT);
            //System_Periodic_Handle();
            pbuf_free(ppkt_buf);
        }
        vTaskDelay(1000);
    }
    udp_remove(udp);
}
int isend = 0;
int ircv = 0;
void test_eth_socket_client_fos(void *pvParameters)
{
    int socket_descriptor = 0;
    struct sockaddr_in address;
    char data_test[500] = {1};
    struct ip_mreq multiCast;
    int i = 0;

    //vTaskDelay(5000);
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.5.196");
    //address.sin_port = htons(8080);
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(7667);
    //Ifx_print1("test_eth_socket_client_fos init \r\n");
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    multiCast.imr_multiaddr.s_addr = inet_addr("239.255.76.67");
    multiCast.imr_interface.s_addr = INADDR_ANY;
    setsockopt(socket_descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&multiCast, sizeof(multiCast));
    //Ifx_print1("test_eth_socket_client_fos down\r\n");

    for (;;)
    {
        //Ifx_print("send -- %d--msg \r\n", i++);
        i++;
        isend++;
        sendto(socket_descriptor, data_test, sizeof(data_test), 0, (struct sockaddr *)&address, sizeof(address));
        if (i % 200 == 0)
        {
            //Ifx_print1("%x--*%d*%d*%d*%d*%d*%d*%d \r\n", eth_flag, i, irq_all, irq_rx, irq_tx, irq_tu, irq_eri, irq_nis);
        }
        vTaskDelay(10);
    }
}

void test_eth_socket_server_fos(void *pvParameters)
{
    int socket_descriptor = 0;
    struct sockaddr_in address;
    char data_test[128] = {0};
    int i = 0;
    err_t err;
    struct ip_mreq multiCast;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(7667);
    Ifx_print("test_eth_socket_server_fos init \r\n");
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    multiCast.imr_multiaddr.s_addr = inet_addr("239.255.76.67");
    multiCast.imr_interface.s_addr = INADDR_ANY;
    setsockopt(socket_descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&multiCast, sizeof(multiCast));

    err = bind(socket_descriptor, (struct sockaddr *)&address, sizeof(address));
    if (err != ERR_OK)
    {
        Ifx_print("bind error.%x \r\n", err);
        while (1)
        {
        }
    }
    Ifx_print("test_eth_socket_server_fos down\r\n");

    for (;;)
    {
        //Ifx_print("recvdata start\r\n");
        err = recv(socket_descriptor, data_test, sizeof(data_test), 0);
        i++;
        ircv++;
        if (i % 20 == 0)
            Ifx_print("-%d -**-%d \r\n", ircv,isend);
        //vTaskDelay(1000);
    }
}
