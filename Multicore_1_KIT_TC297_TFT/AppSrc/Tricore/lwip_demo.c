#include "Pub.h"
#include "lwip_demo.h"

#define UDP_SERVER_PORT 8080

static struct netif xnetif =
    {
        /* set MAC hardware address length */
        .hwaddr_len = (u8_t)ETH_HWADDR_LEN,

        /* set MAC hardware address */
        .hwaddr = {0x00, 0x20, 0x30, 0x40, 0x50, 0x60},

        /* maximum transfer unit */
        .mtu = 1500U,
};

#define ISR_PRIORITY_ETH_IRQ 3

void eth_link_monitor(void *pvParameters)
{
    while (1)
    {
        vTaskDelay(1000);
        xnetif.link_callback(&xnetif);
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
    netif_add(&xnetif, &ipaddr, &netmask, &gw, (void *)0, &ethernetif_init, ethernet_input);

    /**/
    Ifx_print("eth_link_monitor will run %p \r\n", &xnetif);

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
    netif_add(&xnetif, &ipaddr, &netmask, &gw, (void *)0, &ethernetif_init, tcpip_input);

    //netif_set_up(&xnetif);
    Ifx_print("eth_link_monitor will run %p \r\n", &xnetif);
    xTaskCreate(eth_link_monitor, "eth_link_monitor", 1024, NULL, 4, NULL);
    Ifx_print("net_init_socket down \r\n");
}

void eth_init(void *pvParameters)
{
    //net_init();//when test_eth use this
    net_init_socket(); // when test_eth_socket* use this
    ethernetif_recv(&xnetif);
}

IFX_INTERRUPT(ETH_IRQHandler, 0, ISR_PRIORITY_ETH_IRQ);
void ETH_IRQHandler(void)
{
    static BaseType_t xHigherPriorityTaskWoken;
    /* Disable further ETH RX interrupts */
    if (IfxEth_isRxInterrupt(&g_drv_eth.eth))
    {
        //led_107_on();
        IfxPort_setPinState(LED1, IfxPort_State_toggled);
        IfxEth_clearRxInterrupt(&g_drv_eth.eth);
        xSemaphoreGiveFromISR(g_eth_swamphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if (IfxEth_isTxInterrupt(&g_drv_eth.eth))
    {
        //led_108_on();
        IfxEth_clearTxInterrupt(&g_drv_eth.eth);
    }
    if (IfxEth_isTuInterrupt(&g_drv_eth.eth))
    {
        IfxEth_clearTuInterrupt(&g_drv_eth.eth);
    }
    IfxSrc_clearRequest(&SRC_ETH_ETH0_SR);
}

void recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *pkt_buf,
                   const ip_addr_t *addr, u16_t port)
{
    char data[100] = {0};
    Ifx_print("recv_callback in \r\n");
    /* process new connection request */
    memcpy(data, pkt_buf->payload, pkt_buf->len);
    Ifx_print("data is %s \r\n", data);
    pbuf_free(pkt_buf);
}

void test_eth(void *pvParameters)
{
    struct udp_pcb *udp;
    err_t err;
    ip_addr_t dest_addr;
    struct pbuf *ppkt_buf;
    char data_test[10] = {1};
    int i = 0;

    Ifx_print("test_eth init\r\n");
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
    udp_recv(udp, recv_callback, NULL);
    Ifx_print("test_eth down\r\n");

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

void test_eth_socket_client(void *pvParameters)
{
    int socket_descriptor = 0;
    struct sockaddr_in address;
    char data_test[10] = {1};
    int i = 0;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.5.196");
    address.sin_port = htons(8080);
    Ifx_print("test_eth_socket_client init \r\n");
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    Ifx_print("test_eth_socket_client down\r\n");

    for (;;)
    {
        Ifx_print("send -- %d--msg \r\n", i++);
        sendto(socket_descriptor, data_test, sizeof(data_test), 0, (struct sockaddr *)&address, sizeof(address));
        vTaskDelay(1000);
    }
}

void test_eth_socket_server(void *pvParameters)
{
    int socket_descriptor = 0;
    struct sockaddr_in address;
    char data_test[128] = {0};
    int i = 0;
    err_t err;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    Ifx_print("test_eth_socket_server init \r\n");
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    err = bind(socket_descriptor, (struct sockaddr *)&address, sizeof(address));
    if (err != ERR_OK)
    {
        Ifx_print("bind error.%x \r\n", err);
        while (1)
        {
        }
    }
    Ifx_print("test_eth_socket_server down\r\n");

    for (;;)
    {
        Ifx_print("recvdata start\r\n");
        err = recv(socket_descriptor, data_test, sizeof(data_test), 0);
        Ifx_print("recvdata is  -- %d- msg is %s\r\n", i++, data_test);
        vTaskDelay(1000);
    }
}
