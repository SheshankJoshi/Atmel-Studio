/**
 * \file
 *
 * \brief spm source file.
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

#include  "reg.h"

#if defined( __GNUC__ )
//=============================================================================
// Writes one word to a temporary page buffer
.global spmWriteWord
.func spmWriteWord
spmWriteWord:
	push	r0
	push	r1
	movw	r0, r20
	ldi	r25, (1 << SPMEN)	
	rcall	spmSPM
	pop	r1
	pop	r0
	ret
.endfunc
//=============================================================================
// Writes Lock Bits
.global spmWriteLockBits
.func spmWriteLockBits
spmWriteLockBits:
	push	r0
	mov	r0, r24
	ldi	r25, (1 << BLBSET) | (1 << SPMEN)
	rcall	spmSPM
	pop	r0
	ret
.endfunc

//=============================================================================
// Erases one flash page
.global spmErasePage
.func spmErasePage
spmErasePage:
	ldi	r25, (1 << PGERS) | (1 << SPMEN)
	rjmp	spmSPM
.endfunc

//=============================================================================
// Programs the temporary buffer to flash memory
.global spmProgramPage
.func spmProgramPage

spmProgramPage:
	ldi	r25, (1 << PGWRT) | (1 << SPMEN)
	rjmp	spmSPM
.endfunc

//=============================================================================
// Executes self-programming command
.func spmSPM
spmSPM:
	push	r30
	push	r31
	movw	r30, r22
	rcall	spmWait

#ifdef __RAMPZ__
	out	_RAMPZ_, r24
#endif

#ifdef __MEMSPM__
	sts	SPMREG, r25
#else		
	out	SPMREG, r25
#endif
	spm
	nop
	pop	r31
	pop	r30
	ret
	
spmWait:
	push	r25
spmWait_1:
#ifdef __MEMSPM__
	lds	r25, SPMREG
	andi	r25, (1 << SPMEN)
	brne	spmWait_1	
#else
	in	r25, SPMREG
	sbrc	r25, SPMEN
	rjmp	spmWait_1
#endif
	pop	r25
	ret
.endfunc

//=============================================================================
// Writes one byte to EEPROM memory
.global spmEEWriteByte
.func spmEEWriteByte
spmEEWriteByte:
	rcall	spmWait
	rcall	spmEEWriteByteComplete

	out	EEARL, r24
	out	EEARH, r25
	out	EEDR, r22

	sbi	EECR, EEMWE
	sbi	EECR, EEWE
		
spmEEWriteByteComplete:
	sbic	EECR, EEWE
	rjmp	spmEEWriteByteComplete
	ret
.endfunc

#elif defined __IAR_SYSTEMS_ASM__

NAME	spm(16)
PUBLIC	spmWriteWord
PUBLIC	spmWriteLockBits
PUBLIC	spmErasePage
PUBLIC	spmProgramPage
PUBLIC	spmEEWriteByte
RSEG	CODE

//=============================================================================
// Writes one word to a temporary page buffer
spmWriteWord:
	movw	r1:r0, r21:r20
	ldi	r22, (1 << SPMEN)	//need to change
	rjmp	spmSPM
	ret

	

//=============================================================================
// Writes Lock Bits
spmWriteLockBits:
	mov	r0, r16
	ldi	r22, (1 << BLBSET) | (1 << SPMEN)
	rjmp	spmSPM
		

//=============================================================================
// Erases one flash page
spmErasePage:
	ldi	r22, (1 << PGERS) | (1 << SPMEN)
	rjmp	spmSPM


//=============================================================================
// Programs the temporary buffer to flash memory
spmProgramPage:
	ldi	r22, (1 << PGWRT) | (1 << SPMEN)


//=============================================================================
// Executes self-programming command
spmSPM:
	movw	r31:r30, r17:r16
	rcall	spmWait
	in	r20, SREG
	cli
#ifdef __RAMPZ__
	in	r21, RAMPZ
	out	RAMPZ, r18
#endif

#ifdef __MEMSPM__
	sts	SPMREG, r22
#else		
	out	SPMREG, r22
#endif	
	spm		
	dw	0xFFFF
	nop
		
#ifdef __RAMPZ__
	out	RAMPZ, r21
#endif
	
	out	SREG, r20
	ret

spmWait:
#ifdef __MEMSPM__
	lds	r23, SPMREG
	andi	r23, (1 << SPMEN)
	brne	spmWait	
#else
	in	r23, SPMREG
	sbrc	r23, SPMEN
	rjmp	spmWait
#endif
	ret


//=============================================================================
// Writes one byte to EEPROM memory
spmEEWriteByte:
	rcall	spmWait
	rcall	spmEEWriteByteComplete

	out	EEARL, r16
	out	EEARH, r17
	out	EEDR, r18

	sbi	EECR, EEMWE
	sbi	EECR, EEWE
		
spmEEWriteByteComplete:
	sbic	EECR, EEWE
	rjmp	spmEEWriteByteComplete
	ret
END
//=============================================================================

#else
#error Unsupported compiler	
#endif