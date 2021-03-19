/**
 * \file
 *
 * \brief lpm source file.
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

#include "reg.h"
#if defined( __GNUC__ )
//=============================================================================
// Reads one Byte from flash

.global lpmReadByte
.func lpmReadByte
lpmReadByte:
	push	r21
	push	r30
	push	r31
#ifdef __RAMPZ__
	movw	r30, r22
	out	_RAMPZ_, r24
	elpm	r21, Z
#else
	movw	r30, r22
	lpm	r21, Z	
#endif
	mov     r24,r21
	pop	r31
	pop	r30
	pop	r21
	ret
.endfunc

//=============================================================================

#elif defined __IAR_SYSTEMS_ASM__
//=============================================================================
// Reads one Byte from flash
PUBLIC	lpmReadByte
RSEG	CODE

lpmReadByte:
	push	r21
	push	r30
	push	r31
#ifdef __RAMPZ__
	movw	r31:r30, r17:r16
	out	_RAMPZ_, r18
	elpm	r21, Z
#else
	movw	r31:r30, r17:r16
	lpm	r21, Z	
#endif
	mov     r16,r21
	pop	r31
	pop	r30
	pop	r21
	ret
END
//=============================================================================

#else
#error Unsupported compiler	
#endif