/**
 * \file
 *
 * \brief crc source file.
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

//=============================================================================
// Polynome used in CRC calculations

#define	CRC_POLYNOME 0x8005

#if defined( __GNUC__ )
//=============================================================================
.global CRC
.func CRC


//=============================================================================
// CRC calculation routine
CRC:
	ldi	r19, 0x08
	ldi	r20, CRC_POLYNOME & 0xff
	ldi	r21, (CRC_POLYNOME >> 8) & 0xff

CRC_Loop:
	; Rotate left. If MSb is 1 -> divide with Generator-polynome.
	lsl	r22		
	rol	r24		
	rol	r25		
	brcc	CRC_SkipEor

	eor	r24, r20
	eor	r25, r21

CRC_SkipEor:
	dec	r19
	brne	CRC_Loop
	ret

//=============================================================================
.endfunc

#elif defined __IAR_SYSTEMS_ASM__
//=============================================================================
PUBLIC	CRC
RSEG	CODE

//=============================================================================
// CRC calculation routine
CRC:
	ldi	r20, 0x08
	ldi	r21, LOW(CRC_POLYNOME)
	ldi	r22, HIGH(CRC_POLYNOME)

CRC_Loop:
	; Rotate left. If MSb is 1 -> divide with Generator-polynome.
	lsl	r18
	rol	r16
	rol	r17
	brcc	CRC_SkipEor
	eor	r16, r21
	eor	r17, r22

CRC_SkipEor:
	dec	r20
	brne	CRC_Loop
	ret

//=============================================================================
END

#else
#error Unsupported compiler	
#endif