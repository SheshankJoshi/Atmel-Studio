#define F_CPU 16000000
#define BAUD 9600
#define BRC ((F_CPU/16/BAUD)-1)
#include <atmel_start.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <usart_basic.h>
void USART_setup_baud(void);
int main(void)
{
char message[] = "Hello Sheshank";
	
	atmel_start_init();
	//USART_0_initialization(); // Initialized to Transmitter and receiver enabled and 8 bit mode. Odd parity
	//uart_9600();
	//USART_setup_baud();
	UBRR0H=(BRC>>8);
	UBRR0L=BRC;
	uint8_t i;

	while (1)
	{
		for (i=0;i<strlen(message)+1;i++){
			while(USART_0_is_rx_ready()) {_delay_ms(20);};
			printf(message);
			//UDR0=message[i];
			printf("\n\r");
			_delay_ms(50);
		}
		//USART_0_write("\n\r");
		_delay_ms(500);
		
	}
	
}

INTERRUPT


