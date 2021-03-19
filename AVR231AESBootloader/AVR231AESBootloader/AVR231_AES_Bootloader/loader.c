/**
 * \file
 *
 * \brief loader source file.
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
#include "aes.h"
#include "bootldr.h"
#include "bus.h"
#include "crc.h"
#include "loader.h"
#include "lpm.h"
#include "spm.h"
//#include <avr/pgmspace.h>
//#include <avr/wdt.h>

//=============================================================================
// Error Codes

#define ERROR_OK 0x11
#define ERROR_CRC 0x22

//=============================================================================
// Frame types

#define TYPE_EOF 0
#define TYPE_ERASE 1
#define TYPE_PREPARE 2
#define TYPE_DATA 3
#define TYPE_PROGRAM 4
#define TYPE_EEPROM 5
#define TYPE_LOCKBITS 6
#define TYPE_RESET 7

//=============================================================================
// Initial vector for cipher block unchaining (INITIALVECTOR_x are
// defined in 'bootldr.h' which is generated by 'create' tool)
//=============================================================================
// Loader routine. For more information about the format of frames, please
// refer to the Application Note Documentation.

union {
	struct {
		unsigned char m_rxBuffer[BUFFER_SIZE]; // Receive buffer
		unsigned char m_pageBuffer[PAGE_SIZE]; // Page is assembled here before
		                                       // getting programmed to flash mem
	} part1;

#if KEY_COUNT > 0
	struct {
		unsigned char m_tempbuf[256]; // Temp buffer for aesInit.
	} part2;
#endif
} sharedbufs;

#define rxBuffer sharedbufs.part1.m_rxBuffer
#define pageBuffer sharedbufs.part1.m_pageBuffer
#if KEY_COUNT > 0
#define tempbuf sharedbufs.part2.m_tempbuf
#endif
#if KEY_COUNT > 0
#if defined(__GNUC__)
const unsigned char initialVector[16] PROGMEM = {(unsigned char)(INITIALVECTOR_3 >> 24),
                                                 (unsigned char)(INITIALVECTOR_3 >> 16),
                                                 (unsigned char)(INITIALVECTOR_3 >> 8),
                                                 (unsigned char)(INITIALVECTOR_3 >> 0),
                                                 (unsigned char)(INITIALVECTOR_2 >> 24),
                                                 (unsigned char)(INITIALVECTOR_2 >> 16),
                                                 (unsigned char)(INITIALVECTOR_2 >> 8),
                                                 (unsigned char)(INITIALVECTOR_2 >> 0),
                                                 (unsigned char)(INITIALVECTOR_1 >> 24),
                                                 (unsigned char)(INITIALVECTOR_1 >> 16),
                                                 (unsigned char)(INITIALVECTOR_1 >> 8),
                                                 (unsigned char)(INITIALVECTOR_1 >> 0),
                                                 (unsigned char)(INITIALVECTOR_0 >> 24),
                                                 (unsigned char)(INITIALVECTOR_0 >> 16),
                                                 (unsigned char)(INITIALVECTOR_0 >> 8),
                                                 (unsigned char)(INITIALVECTOR_0 >> 0)};
#elif defined(__IAR_SYSTEMS_ICC__)
BOOTFLASH unsigned char initialVector[16] = {(unsigned char)(INITIALVECTOR_3 >> 24),
                                             (unsigned char)(INITIALVECTOR_3 >> 16),
                                             (unsigned char)(INITIALVECTOR_3 >> 8),
                                             (unsigned char)(INITIALVECTOR_3 >> 0),
                                             (unsigned char)(INITIALVECTOR_2 >> 24),
                                             (unsigned char)(INITIALVECTOR_2 >> 16),
                                             (unsigned char)(INITIALVECTOR_2 >> 8),
                                             (unsigned char)(INITIALVECTOR_2 >> 0),
                                             (unsigned char)(INITIALVECTOR_1 >> 24),
                                             (unsigned char)(INITIALVECTOR_1 >> 16),
                                             (unsigned char)(INITIALVECTOR_1 >> 8),
                                             (unsigned char)(INITIALVECTOR_1 >> 0),
                                             (unsigned char)(INITIALVECTOR_0 >> 24),
                                             (unsigned char)(INITIALVECTOR_0 >> 16),
                                             (unsigned char)(INITIALVECTOR_0 >> 8),
                                             (unsigned char)(INITIALVECTOR_0 >> 0)};
#endif
#endif

/*
Loader function
*/
void loader(void)
{
	frameindex_t   frameSize;
	unsigned int   crc;
	unsigned int   mnemonic;
	unsigned char *rxBufferPtr;
	frameindex_t   byteCounter;
	unsigned long  address;
	unsigned char  bits;
	frameindex_t   size;
	unsigned char  type;

	unsigned long  addrCounter;
	unsigned char *pageBufferPtr = 0;
#if KEY_COUNT > 0
	unsigned char chainCipherBlock[16];
#ifdef __RAMPZ__
	/*
	    avr-gcc does not support pointer which is more than 16 bit
	    and below is the workaround for this
	*/
	unsigned long initalVectorAddr = BOOT_ADDR & 0xff0000;
	initalVectorAddr |= (const unsigned int)initialVector;

#else
	unsigned int initalVectorAddr = (const unsigned int)initialVector;
#endif

	unsigned char chainCipherIndex = 0;

	/* move initial vectors to chain cipher block */

	do {
		chainCipherBlock[chainCipherIndex] = lpmReadByte(initalVectorAddr++);
	} while (++chainCipherIndex < sizeof(chainCipherBlock)); // Unchaining

	/* Init AES algorithm. */
	aesInit(tempbuf);
#endif

	/* Prepare bus for incoming frames... */
	// busPrepare();

#if 0               
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = (1 << WDE) | (1 << WDP2) | (1 << WDP0);
#endif

	// Loop forever (the loop is broken by Watchdog when RESET record is
	// received)
	for (;;) {

		// Get the frame size
		frameSize = ((busReceiveByte() << 8) | busReceiveByte());

		// Receive a frame of data from communication interface and calculate
		// its CRC
		{
			rxBufferPtr = rxBuffer;
			byteCounter = frameSize;
			crc         = 0;

			do {
				*rxBufferPtr = busReceiveByte();
				crc          = CRC(crc, *rxBufferPtr++);

				wdt_reset();
			} while (--byteCounter);

			busBusy();
		}

		// CRC is OK?
		if (crc == 0x0000) {
			rxBufferPtr = rxBuffer;

// Decrypt 16 bytes, CRC-bytes are ignored.
#if KEY_COUNT > 0
			{
				frameSize -= 2;
				do {

					wdt_reset();
					aesDecrypt(rxBufferPtr, chainCipherBlock);
					rxBufferPtr += 16;
				} while (frameSize -= 16);
			}
#endif // KEY_COUNT > 0

			rxBufferPtr = rxBuffer;
			// Check that the signature is correct
			if ((*rxBufferPtr++ == (unsigned char)(SIGNATURE >> 24))
			    && (*rxBufferPtr++ == (unsigned char)(SIGNATURE >> 16))
			    && (*rxBufferPtr++ == (unsigned char)(SIGNATURE >> 8))
			    && (*rxBufferPtr++ == (unsigned char)(SIGNATURE >> 0))) {

				// Continue parsing the frames until the 'End Of Frame' is
				// received
				while ((type = *rxBufferPtr++) != TYPE_EOF) {
					wdt_reset();

					// Receive Lock Bits (used in TYPE_LOCKBITS)
					bits = *rxBufferPtr++;

					// Destination Address (note: 'bits' and 'address' overlap)
					address = ((unsigned long)bits << 16);
					address = ((unsigned long)address | ((unsigned long)*rxBufferPtr++ << 8));
					address = ((unsigned long)address | *rxBufferPtr++);

					size = ((unsigned int)*rxBufferPtr++ << 8);
					size |= *rxBufferPtr++;

					if (type == TYPE_ERASE || type == TYPE_PREPARE) {

						if (type == TYPE_ERASE)
							spmErasePage(address);
						// Fall-through!

						// Prepare for incoming data chunks by copying the page
						// contents into the page buffer

						pageBufferPtr = pageBuffer;
						addrCounter   = address;

						do {
							*pageBufferPtr++ = lpmReadByte(addrCounter++);
						} while (--size);
					} else if (type == TYPE_DATA) {
						pageBufferPtr = &pageBuffer[(address & 0xffff)];

						do {

							*pageBufferPtr++ = *rxBufferPtr++;

						} while (--size);

					} else if (type == TYPE_PROGRAM) {
						pageBufferPtr = pageBuffer;
						addrCounter   = address;
						do {
							wdt_reset();
							mnemonic = *(pageBufferPtr + 1);
							mnemonic <<= 8;
							mnemonic |= *(pageBufferPtr);
							spmWriteWord(addrCounter, mnemonic);
							pageBufferPtr += 2;
							addrCounter += 2;
						} while (--size);

						spmErasePage((unsigned long)address);
						spmProgramPage((unsigned long)address);

					} else if (type == TYPE_EEPROM) {
						do {
							wdt_reset();
							spmEEWriteByte((unsigned int)address++ & 0xffff, *pageBufferPtr++);
						} while (--size);
					} else if (type == TYPE_LOCKBITS) {
						spmWriteLockBits(bits);
					} else if (type == TYPE_RESET) {
						busReplyByte(ERROR_OK);
						for (;;)
							;
					} else {
						pageBufferPtr -= 5;
					}
				}
			}

			busReplyByte(ERROR_OK);
		} else {
			busReplyByte(ERROR_CRC);
		}
	}
}