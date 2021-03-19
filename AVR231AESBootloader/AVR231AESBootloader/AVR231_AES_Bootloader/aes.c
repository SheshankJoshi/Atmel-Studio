/**
 * \file
 *
 * \brief aes source file.
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
#include "loader.h"
#include "lpm.h"

#if KEY_COUNT > 0

#include "aeskeys.inc"

typedef unsigned char byte;

#define BPOLY 0x1b   //!< Lower 8 bits of (x^8+x^4+x^3+x+1), ie. (x^4+x^3+x+1).
#define BLOCKSIZE 16 //!< Block size in number of bytes.

#if KEY_COUNT == 1
#define KEYBITS 128 //!< Use AES128.
#elif KEY_COUNT == 2
#define KEYBITS 192 //!< Use AES196.
#elif KEY_COUNT == 3
#define KEYBITS 256 //!< Use AES256.
#else
#error Use 1, 2 or 3 keys!
#endif

#if KEYBITS == 128
#define ROUNDS 10    //!< Number of rounds.
#define KEYLENGTH 16 //!< Key length in number of bytes.
#elif KEYBITS == 192
#define ROUNDS 12    //!< Number of rounds.
#define KEYLENGTH 24 //!< // Key length in number of bytes.
#elif KEYBITS == 256
#define ROUNDS 14    //!< Number of rounds.
#define KEYLENGTH 32 //!< Key length in number of bytes.
#else
#error Key must be 128, 192 or 256 bits!
#endif

#define EXPANDED_KEY_SIZE (BLOCKSIZE * (ROUNDS + 1)) //!< 176, 208 or 240 bytes.

byte block1[256]; //!< Workspace 1.
byte block2[256]; //!< Worksapce 2.

byte *powTbl;      //!< Final location of exponentiation lookup table.
byte *logTbl;      //!< Final location of logarithm lookup table.
byte *sBox;        //!< Final location of s-box.
byte *sBoxInv;     //!< Final location of inverse s-box.
byte *expandedKey; //!< Final location of expanded key.

void CalcPowLog(byte *powTbl, byte *logTbl)
{
	byte i = 0;
	byte t = 1;

	do {
		// Use 0x03 as root for exponentiation and logarithms.
		powTbl[i] = t;
		logTbl[t] = i;
		i++;

		// Muliply t by 3 in GF(2^8).
		t ^= (t << 1) ^ (t & 0x80 ? BPOLY : 0);
	} while (t != 1); // Cyclic properties ensure that i < 255.

	powTbl[255] = powTbl[0]; // 255 = '-0', 254 = -1, etc.
}

void CalcSBox(byte *sBox)
{
	byte i, rot;
	byte temp;
	byte result;

	// Fill all entries of sBox[].
	i = 0;
	do {
		// Inverse in GF(2^8).
		if (i > 0) {
			temp = powTbl[255 - logTbl[i]];
		} else {
			temp = 0;
		}

		// Affine transformation in GF(2).
		result = temp ^ 0x63; // Start with adding a vector in GF(2).
		for (rot = 0; rot < 4; rot++) {
			// Rotate left.
			temp = (temp << 1) | (temp >> 7);

			// Add rotated byte in GF(2).
			result ^= temp;
		}

		// Put result in table.
		sBox[i] = result;
	} while (++i != 0);
}

void CalcSBoxInv(byte *sBox, byte *sBoxInv)
{
	byte i = 0;
	byte j = 0;

	// Iterate through all elements in sBoxInv using  i.
	do {
		// Search through sBox using j.
		do {
			// Check if current j is the inverse of current i.
			if (sBox[j] == i) {
				// If so, set sBoxInc and indicate search finished.
				sBoxInv[i] = j;
				j          = 255;
			}
		} while (++j != 0);
	} while (++i != 0);
}

void InvMixColumn(byte *column)
{
	byte result0, result1, result2, result3;
	byte column0, column1, column2, column3;
	byte xor ;
	byte pos = 0;

	// This generates more effective code, at least
	// with the IAR C compiler.
	column0 = column[0];
	column1 = column[1];
	column2 = column[2];
	column3 = column[3];

	// Partial sums (modular addition using XOR).
	result0 = column1 ^ column2 ^ column3;
	result1 = column0 ^ column2 ^ column3;
	result2 = column0 ^ column1 ^ column3;
	result3 = column0 ^ column1 ^ column2;

// Multiply column bytes by 2 modulo BPOLY.
// This operation is done the following way to ensure cycle count
// independent from data contents. Take care when changing this code.
multiply:
	xor = 0;
	if (column0 & 0x80) {
		xor = BPOLY;
	}
	column0 <<= 1;
	column0 ^= xor;

	xor = 0;
	if (column1 & 0x80) {
		xor = BPOLY;
	}
	column1 <<= 1;
	column1 ^= xor;

	xor = 0;
	if (column2 & 0x80) {
		xor = BPOLY;
	}
	column2 <<= 1;
	column2 ^= xor;

	xor = 0;
	if (column3 & 0x80) {
		xor = BPOLY;
	}
	column3 <<= 1;
	column3 ^= xor;
	if (pos == 1)
		goto more1;
	else if (pos == 2)
		goto more2;

	// More partial sums.
	result0 ^= column0 ^ column1;
	result1 ^= column1 ^ column2;
	result2 ^= column2 ^ column3;
	result3 ^= column0 ^ column3;

	// Multiply column bytes by 2 modulo BPOLY.
	// This operation is done the following way to ensure cycle count
	// independent from data contents. Take care when changing this code.
	pos = 1;
	goto multiply;

// More partial sums.
more1:
	result0 ^= column0 ^ column2;
	result1 ^= column1 ^ column3;
	result2 ^= column0 ^ column2;
	result3 ^= column1 ^ column3;

	// Multiply column bytes by 2 modulo BPOLY.
	// This operation is done the following way to ensure cycle count
	// independent from data contents. Take care when changing this code.
	pos = 2;
	goto multiply;
more2:
	// Final partial sum.
	column0 ^= column1 ^ column2 ^ column3;

	// Final sums stored into original column bytes.
	column[0] = result0 ^ column0;
	column[1] = result1 ^ column0;
	column[2] = result2 ^ column0;
	column[3] = result3 ^ column0;
}

void SubBytes(byte *bytes, byte count)
{
	do {
		*bytes = sBox[*bytes]; // Substitute every byte in state.
		bytes++;
	} while (--count);
}

void InvSubBytesAndXOR(byte *bytes, byte *key, byte count)
{
	do {
		*bytes = block2[*bytes] ^ *key; // Use block2 directly. Increases speed.
		bytes++;
		key++;
	} while (--count);
}

void InvShiftRows(byte *state)
{
	byte temp;

	// Note: State is arranged column by column.

	// Cycle second row right one time.
	temp             = state[1 + 3 * 4];
	state[1 + 3 * 4] = state[1 + 2 * 4];
	state[1 + 2 * 4] = state[1 + 1 * 4];
	state[1 + 1 * 4] = state[1 + 0 * 4];
	state[1 + 0 * 4] = temp;

	// Cycle third row right two times.
	temp             = state[2 + 0 * 4];
	state[2 + 0 * 4] = state[2 + 2 * 4];
	state[2 + 2 * 4] = temp;
	temp             = state[2 + 1 * 4];
	state[2 + 1 * 4] = state[2 + 3 * 4];
	state[2 + 3 * 4] = temp;

	// Cycle fourth row right three times, ie. left once.
	temp             = state[3 + 0 * 4];
	state[3 + 0 * 4] = state[3 + 1 * 4];
	state[3 + 1 * 4] = state[3 + 2 * 4];
	state[3 + 2 * 4] = state[3 + 3 * 4];
	state[3 + 3 * 4] = temp;
}

void InvMixColumns(byte *state)
{
	InvMixColumn(state);
	InvMixColumn(state + 1 * 4);
	InvMixColumn(state + 2 * 4);
	InvMixColumn(state + 3 * 4);
}

void XORBytes(byte *bytes1, byte *bytes2, byte count)
{
	do {
		*bytes1 ^= *bytes2; // Add in GF(2), ie. XOR.
		bytes1++;
		bytes2++;
	} while (--count);
}

void CopyBytes(byte *to, byte *from, byte count)
{
	do {
		*to = *from;
		to++;
		from++;
	} while (--count);
}

void KeyExpansion(byte *expandedKey)
{
	byte temp[5];
	byte Rcon[4] = {0x01, 0x00, 0x00, 0x00}; // Round constant.

#ifdef __RAMPZ__
	/*
	    avr-gcc does not support pointer which is more than 16 bit
	    and below is the workaround for this
	*/
	unsigned long q = BOOT_ADDR & 0xff0000;
	q |= (const unsigned int)kTable;
#else
	unsigned int q = (const unsigned int)kTable;
#endif
	unsigned char i = 0;

	do {
		*expandedKey++ = lpmReadByte(q++);
	} while (++i < KEYLENGTH); // Unchaining

	// Prepare last 4 bytes of key in temp.
	expandedKey -= 4;
	temp[0] = *(expandedKey++);
	temp[1] = *(expandedKey++);
	temp[2] = *(expandedKey++);
	temp[3] = *(expandedKey++);

	// Expand key.
	i = KEYLENGTH;
	while (i < BLOCKSIZE * (ROUNDS + 1)) {
		// Are we at the start of a multiple of the key size?
		if ((i % KEYLENGTH) == 0) {
			temp[4] = temp[0];
			temp[0] = temp[1];
			temp[1] = temp[2];
			temp[2] = temp[3];
			temp[3] = temp[4];

			SubBytes(temp, 4);       // Substitute each byte.
			XORBytes(temp, Rcon, 4); // Add constant in GF(2).
			*Rcon = (*Rcon << 1) ^ (*Rcon & 0x80 ? BPOLY : 0);
		}

// Keysize larger than 24 bytes, ie. larger that 192 bits?
#if KEYLENGTH > 24
		// Are we right past a block size?
		else if ((i % KEYLENGTH) == BLOCKSIZE) {

			SubBytes(temp, 4); // Substitute each byte.
		}
#endif

		// Add bytes in GF(2) one KEYLENGTH away.

		XORBytes(temp, expandedKey - KEYLENGTH, 4);

		// Copy result to current 4 bytes.
		*(expandedKey++) = temp[0];
		*(expandedKey++) = temp[1];
		*(expandedKey++) = temp[2];
		*(expandedKey++) = temp[3];

		i += 4; // Next 4 bytes.
	}
}

void InvCipher(byte *block, byte *expandedKey)
{
	byte round = ROUNDS - 1;
	expandedKey += BLOCKSIZE * ROUNDS;

	XORBytes(block, expandedKey, 16);
	expandedKey -= BLOCKSIZE;

	do {
		InvShiftRows(block);
		InvSubBytesAndXOR(block, expandedKey, 16);
		expandedKey -= BLOCKSIZE;
		InvMixColumns(block);

	} while (--round);

	InvShiftRows(block);
	InvSubBytesAndXOR(block, expandedKey, 16);
}

void aesInit(unsigned char *tempbuf)
{
	powTbl = block1;
	logTbl = block2;
	CalcPowLog(powTbl, logTbl);

	sBox = tempbuf;
	CalcSBox(sBox);

	expandedKey = block1;
	KeyExpansion(expandedKey);

	sBoxInv = block2; // Must be block2.
	CalcSBoxInv(sBox, sBoxInv);
}

void aesDecrypt(unsigned char *buffer, unsigned char *chainBlock)
{
	byte temp[BLOCKSIZE];
	CopyBytes(temp, buffer, BLOCKSIZE);
	InvCipher(buffer, expandedKey);
	XORBytes(buffer, chainBlock, BLOCKSIZE);
	CopyBytes(chainBlock, temp, BLOCKSIZE);
}

#endif
