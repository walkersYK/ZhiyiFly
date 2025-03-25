#include "config.h"
#ifndef _AT_CMD_H
#define _AT_CMD_H
#define AT_RST "AT+RST"//
#define AT_CWMODE "AT+CWMODE=2"//服务器模式
#define AT_CWSAP "AT+CWSAP=\"STM32_WIFI_USART_SOCKET_ANDROID\",\"12345678\",1,3"//热点
#define AT_CIPMUX "AT+CIPMUX=1"//启用多连接
#define AT_CIPSERVER "AT+CIPSERVER=1,8088"//启用服务器端口8088
#define AT_CIFSR "AT+CIFSR"//获取服务器信息
#define AT_CIPSEND "AT+CIPSEND=0,20"//为0号机发送20位数据

extern volatile uint8_t uart_rx_buffer[];
extern volatile uint8_t packet_ready;

void at_init(void);
void ReadRemoteControl(sReceiveRemote *remote);;
#endif
