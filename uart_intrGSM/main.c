// main.c
#include <string.h>
#include "uart.h"

int main()
{
	SystemInit();
	uart_init(9600);
	uart_puts("Hello DESD!!\r\n");
	uart_puts("God Bless You!!\r\n");
	while(1);
	return 0;
}

