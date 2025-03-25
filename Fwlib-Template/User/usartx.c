#include  "usartx.h"
#include  "stdio.h"
	static void NVIC_Configuration(){
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	void USART_Config(){
	  GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		USART_GPIO_APBxClkCmd(USART_GPIO_CLK,ENABLE);
		USART_APBxClkCmd(USART_CLK,ENABLE);
		
		GPIO_InitStructure.GPIO_Pin=USART_TX_GPIO_Pin;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(USART_TX_GPIO_PORT,&GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin=USART_RX_GPIO_Pin;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
		GPIO_Init(USART_RX_GPIO_PORT,&GPIO_InitStructure);
		//配置串口工作参数
		//波特率
		USART_InitStructure.USART_BaudRate=USART_BAUDRATE;
		//帧数据字长
		USART_InitStructure.USART_WordLength=USART_WordLength_8b;
		//配置停止位
		USART_InitStructure.USART_StopBits=USART_StopBits_1;
		//校验位
		USART_InitStructure.USART_Parity=USART_Parity_No;
		//硬件流控制
		USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
		//工作模式 
		USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
		//初始化
	  USART_Init(USARTx,&USART_InitStructure);
		//串口中断优先级配置
	  NVIC_Configuration();
		//使能串口接收中断
		USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
		//使能串口
		USART_Cmd(USARTx,ENABLE);
	}
	void USARTx_SendByte(uint8_t ch){
		//发送一个字节
		USART_SendData(USARTx,ch);
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE)==RESET);
	}
	void USARTx_SendString(char* str){
	unsigned int k=0;
		do{
		USARTx_SendByte(*(str+k));
			k++;
		}while(*(str+k)!='\0');
		while (USART_GetFlagStatus(USARTx,USART_FLAG_TC)==RESET);
	}
	void USARTx_SendPacket(const void *data, uint8_t len) {
    const uint8_t *p = data;
    // 添加包头
    USARTx_SendByte(0xAA); 
    USARTx_SendByte(len);
    // 发送数据
    for(int i=0; i<len; i++) {
        USARTx_SendByte(p[i]);
    }
    // 添加校验
    uint8_t checksum = 0xAA + len;
    for(int i=0; i<len; i++) checksum += p[i];
    USARTx_SendByte(checksum);
}
	
	/*调试*/
int fputc(int ch, FILE *f)
{
	USARTx_SendByte(ch);
	return ch;
}

//void USART2_IRQHandler(void) {
//    static uint8_t rxBuffer[20];
//    static uint8_t index = 0;
//    
//    if(USART_GetITStatus(USART2, USART_IT_RXNE)) {
//        rxBuffer[index++] = USART_ReceiveData(USART2);
//        
//        if(index >= sizeof(sReceiveRemote)) {
//            sReceiveRemote *remote = (sReceiveRemote*)rxBuffer;
//            processControlData(remote);
//            index = 0;
//        }
//    }
//}

//void processControlData(sReceiveRemote *data) {
//    // 示例：驱动电机和舵机
//    int16_t throttle = data->THR; // -32768~32767
//    int16_t steering = data->YAW;
//    
//    // 转换为实际控制量（示例）
//    float motorSpeed = (float)throttle / 32767.0f; // 归一化到-1~1
//    float steeringAngle = (float)steering / 32767.0f * 45.0f; // 转角度数
//}


