/*
 * Calculation of chains for A5/1 rainbow table cracking.
 *
 *
 * Loosely based on: A pedagogical implementation of A5/1.
 *
 * Copyright (C) 1998-1999: Marc Briceno, Ian Goldberg, and David Wagner
 *
 * See accompanying file A5.1.c for original version and full copyright
 */


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

bit parity32(uint32 x) {
	x ^= x>>16;
	x ^= x>>8;
	x ^= x>>4;
	x ^= x>>2;
	x ^= x>>1;
	return x&1;
}

bit parity64(uint64 x) {
	x ^= x>>32;
	x ^= x>>16;
	x ^= x>>8;
	x ^= x>>4;
	x ^= x>>2;
	x ^= x>>1;
	return x&1;
}

uint32 clockone(uint32 reg, uint32 mask, uint32 taps) {
	uint32 t = reg & taps;
	reg = (reg << 1) & mask;
	reg |= parity32(t);
	return reg;
}

uint32 R1, R2, R3;

inline bit majority() {
	int sum;
	sum = ((R1&R1MID) >> 8) + ((R2&R2MID) >> 10) + ((R3&R3MID) >> 10);
	if (sum >= 2)
		return 1;
	else
		return 0;
}

inline void clock() {
	bit maj = majority();
	if (((R1&R1MID)!=0) == maj)
		R1 = clockone(R1, R1MASK, R1TAPS);
	if (((R2&R2MID)!=0) == maj)
		R2 = clockone(R2, R2MASK, R2TAPS);
	if (((R3&R3MID)!=0) == maj)
		R3 = clockone(R3, R3MASK, R3TAPS);
}

inline bit getbit() {
  return ((R1&R1OUT) >> 18) ^ ((R2&R2OUT) >> 21) ^ ((R3&R3OUT) >> 22);
}

inline uint64 calculate_link (uint64 input, uint32 count) {
  uint64 result;
  int i;

  /* Reduction function. */
  R1 = ((input >> (22 + 23))^count) & R1MASK;
  R2 = ((input >> 23)^count) & R2MASK;
  R3 = (input^count) & R3MASK;

  result = getbit();
  for(i=1;i<64;i++) {
    // Yes, virginia, we only need to clock 63 times for 64 bits of output
    clock();
    result = (result << 1)| getbit();
  }
  return result;
}

uint64 calculate_chain (uint64 input, uint32 count) {
  int i;

  for(i=count-1; i>=0; i--) {
    input = calculate_link(input, i);
  }
  return input;
}

/* Linear congruential generator used to whiten start value.
 * Numbers stolen from Lawrence Livermore National Laboratory RNG
 * library: http://nuclear.llnl.gov/CNP/rng/rngman/node4.html 
 */
uint64 start_value_from_index (uint64 count) {
  return 2862933555777941757UL * (count + 1) + 3037000493L;
}

void usage (char* argv[]) {
  printf("Usage: %s <startvalue>\n\nSpecify the start value as 16 hex digits, or chain number as 10 hex digits. Calculates one\nchain of the A5/1 rainbow table, and outputs the end value.\n", argv[0]);
  exit(1);
}

int main(int argc, char* argv[]) {
  int i,j;
  uint64 current;

  if (argc<2) {
    usage(argv);
  } else if (strlen(argv[1])==16 && sscanf(argv[1], "%16lx", &current) == 1) {
    printf("Calculating chain from start value 0x%16.16lx\n", current);
    current = calculate_chain(current, pow(2, 21));
  } else if (sscanf(argv[1], "%16lx", &current) == 1) {
    current = start_value_from_index(current);
    printf("Calculating chain from start value 0x%16.16lx\n", current);
    current = calculate_chain(current, pow(2, 21));
  } else {
    usage(argv);
  }
  printf("End value: 0x%16.16lx\n", current);
  return 0;
}
