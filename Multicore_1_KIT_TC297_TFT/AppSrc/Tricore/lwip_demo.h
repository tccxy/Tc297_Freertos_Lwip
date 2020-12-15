#ifndef _LWIP_DEMO_H_
#define _LWIP_DEMO_H_

#define UDP_SERVER_PORT 8080


void eth_init_fos(void *pvParameters);
void test_eth_socket_client_fos(void *pvParameters);
void test_eth_socket_server_fos(void *pvParameters);
void test_eth_fos(void *pvParameters);
void test_eth_bare(void);

#endif
