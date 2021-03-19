/**
 * \file
 *
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
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
 
#if defined __GNUC__
#include <avr/pgmspace.h>
const unsigned char kTable[32] PROGMEM = 
{
	0xec, 0x71, 0xb3, 0x26, 0x11, 0xb7, 0x05, 0x0a,
	0x1a, 0xbc, 0xd1, 0x54, 0x72, 0x55, 0x6b, 0x9b,

	0x74, 0x86, 0x0d, 0x78, 0xa4, 0x83, 0xad, 0x0c,

	0xbc, 0x5d, 0x2f, 0x06, 0x47, 0x28, 0xcf, 0x68,
};

#elif defined __IAR_SYSTEMS_ICC__
__flash unsigned char kTable[32] = 
{
	0xec, 0x71, 0xb3, 0x26, 0x11, 0xb7, 0x05, 0x0a,
	0x1a, 0xbc, 0xd1, 0x54, 0x72, 0x55, 0x6b, 0x9b,

	0x74, 0x86, 0x0d, 0x78, 0xa4, 0x83, 0xad, 0x0c,

	0xbc, 0x5d, 0x2f, 0x06, 0x47, 0x28, 0xcf, 0x68,
};
#endif
