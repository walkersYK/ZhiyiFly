#include "config.h"
#ifndef _AT_CMD_H
#define _AT_CMD_H
#define AT_RST "AT+RST"//
#define AT_CWMODE "AT+CWMODE=2"//������ģʽ
#define AT_CWSAP "AT+CWSAP=\"STM32_WIFI_USART_SOCKET_ANDROID\",\"12345678\",1,3"//�ȵ�
#define AT_CIPMUX "AT+CIPMUX=1"//���ö�����
#define AT_CIPSERVER "AT+CIPSERVER=1,8088"//���÷������˿�8088
#define AT_CIFSR "AT+CIFSR"//��ȡ��������Ϣ
#define AT_CIPSEND "AT+CIPSEND=0,20"//Ϊ0�Ż�����20λ����

extern volatile uint8_t uart_rx_buffer[];
extern volatile uint8_t packet_ready;

void at_init(void);
void ReadRemoteControl(sReceiveRemote *remote);;
#endif
