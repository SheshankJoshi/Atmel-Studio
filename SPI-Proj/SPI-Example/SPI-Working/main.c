#include <atmel_start.h>
#include <spi_basic.h>
#include <util/delay.h>
#include <string.h>
#include <stddef.h>
#include <src/spi_basic.c>

typedef struct message
	{
		
		uint8_t connection_test;
		uint8_t connection_test_received;
		char Initial_Message [] ="Initialization Done";
	};
	


struct message Message;



bool SPI_Makeready(void);
int error_function_for_spi(int code);


int main(void)
{
	atmel_start_init();
	
	Message.connection_test=20;
	//Message.Initial_Message="Initialized better";
	
	char k[] ="Sheshank";
	if(~SPI_Makeready()){return 0;}
		
	

	/* Replace with your application code */
	while (1) {
		SPI_exchange_complete(k);
	}
}
bool SPI_Makeready()
{ /* Making our SPI Application Ready*/
	while(~SPI_0_status_free()){;}
	SPI_0_enable();
	while(~SPI_0_status_idle());
	SPI_0_init();
	Message.connection_test_received=SPI_0_exchange_byte(Message.connection_test);
	if ((Message.connection_test_received==Message.connection_test))
		{
			SPI_0_status_done();
			SPI_0_status_idle();
		}
	else{error_function_for_spi(1);} //Not able to establish the connection
	int status = SPI_exchange_complete(Message.Initial_Message); //Welcome Message and sign of good progress.
	if (status==1){return true;}
	else{return false;}		
}
int SPI_exchange_complete(char *data_to_send)
{
	//SPI_0_register_callback(f);
	char temp[];
	uint8_t length=sizeof(data_to_send);
	while(SPI_0_status_busy());
	
	SPI_0_write_block(data_to_send,length);
	for (int i=0;i<5;i++)
	{
		/*for (j=0;j<length+1;j++){
			SPI_0_exchange_byte()

			}*/
			SPI_0_exchange_block(data_to_send,length);
			while(~SPI_0_status_busy() & SPI_0_status_idle())
			{
				
			}
			temp=SPI_0_read_block(SPI_0_desc.data,sizeof(SPI_0_desc.data));
			if (strcmp(temp,data_to_send)==0)
			{
				SPI_0_status_done();
				SPI_0_status_idle();
				break;
			}
	}
	if (~SPI_0_status_done())
	{
			error_function_for_spi(2); // Transaction even after 5 attempts
			/* More code to add here.*/
	}
	else 
	{
	return 1;		
	}
		//SPI_0_register_callback(t);
	
}

