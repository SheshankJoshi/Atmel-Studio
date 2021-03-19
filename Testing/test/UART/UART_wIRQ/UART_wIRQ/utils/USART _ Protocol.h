#ifndef
#define Buffersize 256
#endif

volatile typedef struct USART_data {
	unsigned char temp_buffer;
	bool command_flag;
	bool data_flag;
	unsigned char txcommand;
	unsigned char rxcommand;
	uint8_t passage_length=0; // This is to be sent along with the data indicating bits for the full frame.
	int trans_count=0;
	int receipt_count=0;
	int trans_frame=0;
	int receipt_frame=0;
	volatile char txBuffer[Buffersize+1];
	volatile char rxBuffer[Buffersize+1];	
	} Uart_block;

volatile typedef struct USART_frame {
	unsigned char[8] characters;
	} FRAME;

volatile struct cmd {
	unsigned char command;
	} cmd;

volatile union buffer {
	FRAME;
	cmd;
} buffer;

	
	
static enum commands {
	const unsigned char sending_command=0xB0;
	const unsigned char sending_data=0xB1;
	
} commands;
	
static enum commands_in{
	// MCU side commands that it receives as a slave and what they mean to the MCU
	const unsigned char sendplease = 0xC1; // Do the processing and send the appropriate information
	const unsigned char keepit = 0xC2; //command asking the device to keep what it is about to send
	const unsigned char wrong_cmd_rcvd = 0xC5 // If the received command is not a right command.
	} commands_in ;
static typedef enum commands_out{
	// PC side or the other side that is sent by our MCU to the host.
	const unsigned char hereitis = 0xC3; // 
	const unsigned char keeping = 0xC4;
	const unsigned char wrong_cmd_sent=0xC6;
} commands_out;
	





void USART_SETUP(void){
	Uart_block Uart;
	commands_out Uart_out;
	commands_in Uart_in;
	//static bool rx_command_immediate;
	//static bool tx_command_immediate;
	
	//sei();
	/*Set up code here so that you will have the appropriate data to send or to receive.*/
	
}
ISR (USART_RX_vect){
	UCSR0B| = 0 << RXCIE0 // To disable reception interrupt while we process this.
	UDR0=Uart_block.temp_buffer; // Loading it into a temporary buffer for analysis.
	int character_length;
////////////////////////////////////////////////////////////////////////////////////////////////////////
//Command preparation block
///////////////////////////////////////////////////////////////////////////////////////////////////////
	if ((Uart_block.command_flag) && !(Uart_block.data_flag)) // putting the received info as a command. // length of command not decided.
	{
		Uart_block.rxcommand=Uart_block.temp_buffer; //deal with command flag here.
		/////////////////////////////////////////////////////////////////////////////////////
		if ((Uart_block.command_flag) && !(Uart_block.data_flag)) //Execute command stored at rxcommand. Write a switch case for various commands.
		{
			/* Put the code here that executes the particular command. Or write a function and jump to that place and raises a flag when its job is done.*/
		}
		/* Write some code here for analysis for calling some other function or something.*/
		//Uart_block.command_flag=false;
		UCSR0B| = 1 << RXCIE0;
		return;	
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//End of command preparation Block
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Data preparation Block
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (Uart_block.data_flag && (Uart_block.passage_length==0))
	{
		Uart_block.rxcommand=commands_in.keepit;
		Uart_block.passage_length=Uart_block.temp_buffer;
		Uart_block.passage_length==0;
		Uart_block.command_flag=false;
		UCSR0B| = 1 << RXCIE0;
		return;
	}	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//End of Data preparation stage
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Entry Byte (First Byte) Handling Block
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (Uart_block.temp_buffer==commands.sending_command) // If it is indicated that it is going to send a command or not. To differentiate from random and other useless comms.Prepare if sending commmand
	{
		Uart_block.rxcommand=commands.sending_command;
		Uart_block.command_flag=true;
		Uart_block.data_flag=false;
		//Uart_block.temp_buffer=0x00;
		UCSR0B| = 1 << RXCIE0; // Re-enabling the reception interrupt.
		return;
	}
	else if (Uart_block.temp_buffer==commands.sending_data)
	{
		Uart_block.rxcommand=commands_in.keepit;
		Uart_block.data_flag=true;
		Uart_block.passage_length==0;
		//Uart_block.temp_buffer=0x00;
		UCSR0B| = 1 << RXCIE0;
		return;	
	}
	else
	{
			Uart_block.txcommand=commands_in.wrong_cmd_rcvd;
			//tx_command_immediate=true;
			Uart_block.command_flag=false;
			Uart_block.data_flag=false;
			//Uart_block.temp_buffer=0x00;
			UCSR0B| = 1 << RXCIE0;
			/*Point to the function that sends the immediate character indicating the same or raise an interrupt that send the data directly*/
			return;
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End of first bytes handling block.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Data Handling Block
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if ((Uart_block.data_flag) & !(Uart_block.command_flag)) //for double checking, ignore this.
	{
		/////////////////////////////////////////////////////////////////////////////////////
		if ((Uart_block.rxcommand==commands_in.keepit) && (Uart_block.passage_length==0)) // indicating what we have just received is the length of the file we are about to receive.
		{
			character_length=Uart_block.temp_buffer; // setting the number of data bytes that is to follow
			Uart_block.passage_length=((int)character_length); // Handle the exception case where the length is not exactly divisible by 8 later on.
		}
		/////////////////////////////////////////////////////////////////////////////////////
			/* Write the Code here such that you will store the data somewhere or write the data. But here we are going
			to use it to fill the buffer. Also check the data flag and poll for any change at all, if no change observed that means data ended.*/
		else if((Uart_block.passage_length!=0)) //figure out the condition here.
		{
			if (Uart_block.receipt_frame==Buffersize/8) // change the code here for buffer full.
			{
				/* Write some code here to indicate that particular buffer is full. After buffer is handled by the interrupt, go back to filling the buffer.*/
				
			}
			else if(Uart_block.passage_length>8)
			{
				FRAME[Uart_block.trans_count]=Uart_block.temp_buffer; //storing the character in frame.
				Uart_block.trans_count++;
				////////////////////////////////////////
				if (Uart_block.trans_count>8)
					{
						Uart_block.trans_count=0; //Indicating that our Frame is full of 8 characters.
						for (int i=0;i<8;i++)
						{
							Uart_block.rxBuffer[Uart_block.receipt_frame+i]=FRAME[i];	// Flushing the entire frame into the buffer character by character.
						}
						Uart_block.receipt_frame+=8;
						Uart_block.passage_length-=8; // Reducing from the whole passage_length that we created out of it.	//Finally use this to inidcate end of transmission.
					}
				///////////////////////////////////////		
			}
			else
			{
				FRAME[Uart_block.trans_count]=Uart_block.temp_buffer; //storing the character in frame.
				Uart_block.trans_count++;
				int temp = Uart_block.passage_length%8;
				///////////////////////////////////////////////////////////////////////////
				if (Uart_block.trans_count>8)
				{
					Uart_block.trans_count=0; //Indicating that our Frame is full of 8 characters.
					for (int i=0;i<temp;i++)
					{
						Uart_block.rxBuffer[Uart_block.receipt_frame+i]=FRAME[i];	// Flushing the entire frame into the buffer character by character.
					}
					Uart_block.receipt_frame==0;
					Uart_block.passage_length==0; // Reducing from the whole passage_length that we created out of it.	//Finally use this to inidcate end of transmission.
				}
				///////////////////////////////////////////////////////////////////////////
				/*Write the code here to signal end of transmission routine for data.*/
			}
		}
		UCSR0B| = 1 << RXCIE0;
		return
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//End of Data Handling Block		
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
}
