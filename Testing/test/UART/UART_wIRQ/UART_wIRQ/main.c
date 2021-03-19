#include <atmel_start.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <util/delay.h>
#include <usart_basic.h>
#include <USART_Protocol.h>
#define Buffersize 256 // Set the buffer size here.
char Message[8] = "sheshank";

int main(void)
{
	atmel_start_init();
	USART_SETUP();
	int i;
	while (1) 
	{
		//printf("hello Sheshank");
		//printf("\n");
			while (USART_0_is_tx_ready())
			{
				for (i=0;i<strlen(Message)-1;i++)
				{
				UDR0=Message[i];
				//printf("Sheshank");
				_delay_ms(1000);
				}
				
				UDR0=0xA;
				_delay_ms(20);
				UDR0=0xD;
				_delay_ms(20);
			}
		
		_delay_ms(1000);
	}
	;
}