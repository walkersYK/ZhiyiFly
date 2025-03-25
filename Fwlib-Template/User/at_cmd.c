#include"at_cmd.h"
#include"usartx.h"
#include <string.h>
#include <stdio.h>

void ReadRemoteControl(sReceiveRemote *remote) 
{
    if (packet_ready) {
        __disable_irq();
        memcpy(remote, (void*)uart_rx_buffer, sizeof(sReceiveRemote));
        packet_ready = 0;
        __enable_irq();
        
        printf("THR:%d YAW:%d ROL:%d PIT:%d\n", 
              remote->THR, remote->YAW, remote->ROL, remote->PIT);
    }
}

void at_init(){
//USARTx_SendString("AT+CWMODE=2");//ģʽ
//USARTx_SendString("AT+RST");//��λ
//USARTx_SendString("AT+CWSAP=\"STM32_WIFI\",\"12345678\",1,3");//�ȵ���Ϣ
USARTx_SendString("AT+CIPMUX=1");//������
USARTx_SendString("AT+CIPSERVER=1,8088");//�˿�
}
