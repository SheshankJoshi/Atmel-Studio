/**
 * \file
 *
 * \brief usart source file.
 *
 * Copyright (c) 2010-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "avr_compiler.h"
#include "bus.h"

#define FOSC 16000000UL // 16 MHz
#define BAUD 38400
#define UBRR_VALUE (FOSC / 16 / BAUD - 1)
#define UBRRL_VALUE (UBRR_VALUE & 0xff)
#define UBRRH_VALUE (UBRR_VALUE >> 8)

//=============================================================================
// Initialize bus interface

void busInit(void)
{
	// 115200 baud @ 3.6864MHz
	// UBRRL = 1;

	// USART0
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	// Enable RX and TX
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	// 8-bit, 1 stop bit, no parity, asynchronous UART
	// UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (0 << USBS0) |
	//(0 << UPM01) | (0 << UPM00) | (0 << UMSEL01) | (0 << UMSEL00);
}

//=============================================================================
// Prepare bus (in this case, wait for the first byte to arrive).

void busPrepare(void)
{
	// Wait for incoming data
	while (!(UCSR0A & (1 << RXC0)))
		;
}

//=============================================================================
// Reply byte

void busReplyByte(unsigned char data)
{
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1 << UDRE0)))
		;
	// Put data into buffer, sends the data
	UDR0 = data;
	// Wait for transmit complete
	while (!(UCSR0A & (1 << TXC0)))
		;
	// Clear TXC flag
	UCSR0A |= 1 << TXC0;
}

//=============================================================================
// Receive byte

unsigned char busReceiveByte(void)
{
	busPrepare();

	return UDR0;
}

//=============================================================================
// Set bus to a busy state. Requires no action in USART.

void busBusy(void)
{
	return;
}
