/*
 * Calculation of chains for A5/1 rainbow table cracking.
 *
 *
 * Loosely based on: A pedagogical implementation of A5/1.
 *
 * Copyright (C) 1998-1999: Marc Briceno, Ian Goldberg, and David Wagner
 *
 * See accompanying file A5.1.c for original version and full copyright
 *
 *
 * Modified and optimized for running on CUDA.
 *
 * Copyright (C) 2009: Ingo Albrecht <prom@berlin.ccc.de>
 *
 */

// XXX: Undefine for 64bit platform. You will also have to fix up printfs.
#define BITSIZE_32

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Masks for the three shift registers */
#define R1MASK	0x07FFFF /* 19 bits, numbered 0..18 */
#define R2MASK	0x3FFFFF /* 22 bits, numbered 0..21 */
#define R3MASK	0x7FFFFF /* 23 bits, numbered 0..22 */

/* Middle bit of each of the three shift registers, for clock control */
#define R1MID	0x000100 /* bit 8 */
#define R2MID	0x000400 /* bit 10 */
#define R3MID	0x000400 /* bit 10 */

/* Feedback taps, for clocking the shift registers. */
#define R1TAPS	0x072000 /* bits 18,17,16,13 */
#define R2TAPS	0x300000 /* bits 21,20 */
#define R3TAPS	0x700080 /* bits 22,21,20,7 */

/* Output taps, for output generation */
#define R1OUT	0x040000 /* bit 18 (the high bit) */
#define R2OUT	0x200000 /* bit 21 (the high bit) */
#define R3OUT	0x400000 /* bit 22 (the high bit) */

typedef unsigned char byte;
#ifdef BITSIZE_32
typedef unsigned long uint32;
typedef unsigned long long uint64;
#else
typedef unsigned int  uint32;
typedef unsigned long uint64;
#endif

typedef unsigned int bit;

__device__ bit parity32(uint32 x) {
	x ^= x>>16;
	x ^= x>>8;
	x ^= x>>4;
	x ^= x>>2;
	x ^= x>>1;
	return x&1;
}

__device__ uint32 clockone(uint32 reg, uint32 mask, uint32 taps) {
	uint32 t = reg & taps;
	reg = (reg << 1) & mask;
	reg |= parity32(t);
	return reg;
}

__device__ bit majority(uint32 R1, uint32 R2, uint32 R3) {
	int sum;
	sum = ((R1&R1MID) >> 8) + ((R2&R2MID) >> 10) + ((R3&R3MID) >> 10);
	if (sum >= 2)
		return 1;
	else
		return 0;
}

__device__ bit getbit(uint32 R1, uint32 R2, uint32 R3) {
  return ((R1&R1OUT) >> 18) ^ ((R2&R2OUT) >> 21) ^ ((R3&R3OUT) >> 22);
}

__device__ uint64 calculate_link (uint64 input, uint32 count) {
  uint64 result;
  int i;

  /* Reduction function. */
  uint32 R1 = ((input >> (22 + 23))^count) & R1MASK;
  uint32 R2 = ((input >> 23)^count) & R2MASK;
  uint32 R3 = (input^count) & R3MASK;

  result = getbit(R1, R2, R3);
  for(i=1;i<64;i++) {
    // Yes, virginia, we only need to clock 63 times for 64 bits of output

    // clock()
    bit maj = majority(R1, R2, R3);
    uint32 T1 = clockone(R1, R1MASK, R1TAPS);
    uint32 T2 = clockone(R2, R2MASK, R2TAPS);
    uint32 T3 = clockone(R3, R3MASK, R3TAPS);

    if (((R1&R1MID)!=0) == maj)
      R1 = T1;
    if (((R2&R2MID)!=0) == maj)
      R2 = T2;
    if (((R3&R3MID)!=0) == maj)
      R3 = T3;

    result = (result << 1)| getbit(R1, R2, R3);
  }
  return result;
}

__global__ void crunch(uint64* results, uint32 index) {
  uint32 tid = blockIdx.x * blockDim.x + threadIdx.x;

  uint64 state = results[tid];

  uint32 i;
  for(i = 0; i < OPERATIONS_PER_RUN; i++) {
    state = calculate_link(state, index - i);
  }
  
  results[tid] = state;
}

