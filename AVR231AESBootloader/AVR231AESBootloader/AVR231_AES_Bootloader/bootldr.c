/**
 * \file
 *
 * \brief bootldr source file.
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
#include "bootldr.h"
#include "bus.h"
#include "crc.h"
#include "loader.h"
#include "lpm.h"
//#include <avr/wdt.h>

//=============================================================================
// Starts here!

int main(void)
{

	busInit();
	// Loop forever (loops only if Application Section is damaged)

	for (;;) {
		// Key pressed? Yes -> run the loader routine
		if (!(PINB & (1 << PINB7)))
			loader();
		else

#ifdef CRC_CHECK
		// Check that the Application Section contents is undamaged
		// by calculating the CRC of the whole memory.
		{
			unsigned int  crc = 0;
			unsigned long n   = 0;

			while (n < MEM_SIZE) {
				crc = CRC(crc, lpmReadByte(n++));
			}

			// Application Section damaged
			//   -> do not jump to Reset Vector of the Application Section
			if (crc) {
				while (1)
					;
			}
		}
#endif
#if MEM_SIZE > (128 * 1024)
		asm("ldi r30,0x00\n ldi r31,0x00\n out 0x3c, r30\n eicall ");
#else
		((void (*)())0x0000)();
#endif
	}
}
