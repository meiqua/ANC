#include <stdio.h>
#include <stm32f4xx.h>
struct __FILE {
    int dummy;
};
FILE __stdout;
int fputc(int ch, FILE *f){
//HAL_UART_Transmit_IT(&UartHandle, (uint8_t*)ch,1);
	ITM_SendChar(ch); 
	return ch;
}

