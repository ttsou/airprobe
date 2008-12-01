/* This file was taken from gsm-tvoid */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <math.h>

//#include "burst_types.h"
#include "conv.h"
//#include "fire_crc.h"


/*
 * Convolutional encoding and Viterbi decoding for the GSM CCH+TCH channel.
 */

/* The class 1 bits are encoded with the 1/2 rate convolutional code defined by
 * the polynomials:
 *	G0 = 1 + D3+ D4
 *	G1 = 1 + D + D3+ D4
 * The coded bits {c(0), c(1),..., c(455)} are then defined by:
 * class 1: c(2k) = u(k) + u(k-3) + u(k-4)
 *        c(2k+1) = u(k) + u(k-1) + u(k-3) + u(k-4)       for k = 0,1,...,188
 *                  u(k) = 0 for k < 0
 * class 2:c(378+k) = d(182+k)                            for k = 0,1,....,77
 */


/*
 * Convolutional encoding:
 *
 *	G_0 = 1 + x^3 + x^4
 *	G_1 = 1 + x + x^3 + x^4
 *
 * i.e.,
 *
 *	c_{2k} = u_k + u_{k - 3} + u_{k - 4}
 *	c_{2k + 1} = u_k + u_{k - 1} + u_{k - 3} + u_{k - 4}
 */
#define K		5
#define MAX_ERROR(size)	(2 * size + 1)


/*
 * Given the current state and input bit, what are the output bits?
 *
 * 	encode[current_state][input_bit]
 */
static const unsigned int encode[1 << (K - 1)][2] = {
	{0, 3}, {3, 0}, {3, 0}, {0, 3},
	{0, 3}, {3, 0}, {3, 0}, {0, 3},
	{1, 2}, {2, 1}, {2, 1}, {1, 2},
	{1, 2}, {2, 1}, {2, 1}, {1, 2}
};


/*
 * Given the current state and input bit, what is the next state?
 * 
 * 	next_state[current_state][input_bit]
 */
static const unsigned int next_state[1 << (K - 1)][2] = {
	{0, 8}, {0, 8}, {1, 9}, {1, 9},
	{2, 10}, {2, 10}, {3, 11}, {3, 11},
	{4, 12}, {4, 12}, {5, 13}, {5, 13},
	{6, 14}, {6, 14}, {7, 15}, {7, 15}
};


/*
 * Given the previous state and the current state, what input bit caused
 * the transition?  If it is impossible to transition between the two
 * states, the value is 2.
 *
 * 	prev_next_state[previous_state][current_state]
 */
static const unsigned int prev_next_state[1 << (K - 1)][1 << (K - 1)] = {
        { 0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  2},
        { 0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  2},
        { 2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2},
        { 2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2},
        { 2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2},
        { 2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2},
        { 2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2},
        { 2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2},
        { 2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2},
        { 2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2},
        { 2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2},
        { 2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2},
        { 2,  2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2},
        { 2,  2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2},
        { 2,  2,  2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1},
        { 2,  2,  2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1}
};


static inline unsigned int hamming_distance2(unsigned int w) {

	return (w & 1) + !!(w & 2);
}


/*
static void conv_encode(unsigned char *data, unsigned char *output,
			unsigned int input_size) {

	unsigned int i, state = 0, o;

	// encode data
	for(i = 0; i < input_size; i++) {
		o = encode[state][data[i]];
		state = next_state[state][data[i]];
		*output++ = !!(o & 2);
		*output++ = o & 1;
	}
}
 */


int
conv_decode(unsigned char *output, unsigned char *data,
	    unsigned int input_size) {

	int i, t;
	unsigned int rdata, state, nstate, b, o, distance, accumulated_error,
	   min_state, min_error, cur_state;

	unsigned int max_error = MAX_ERROR(input_size);
	unsigned int ae[1 << (K - 1)];
	unsigned int nae[1 << (K - 1)]; // next accumulated error
	unsigned int state_history[1 << (K - 1)][CONV_MAX_INPUT_SIZE + 1];

	// initialize accumulated error, assume starting state is 0
	for(i = 0; i < (1 << (K - 1)); i++)
		ae[i] = nae[i] = max_error;
	ae[0] = 0;

	// build trellis
	for(t = 0; t < input_size; t++) {

		// get received data symbol
		rdata = (data[2 * t] << 1) | data[2 * t + 1];

		// for each state
		for(state = 0; state < (1 << (K - 1)); state++) {

			// make sure this state is possible
			if(ae[state] >= max_error)
				continue;

			// find all states we lead to
			for(b = 0; b < 2; b++) {

				// get next state given input bit b
				nstate = next_state[state][b];

				// find output for this transition
				o = encode[state][b];

				// calculate distance from received data
				distance = hamming_distance2(rdata ^ o);

				// choose surviving path
				accumulated_error = ae[state] + distance;
				if(accumulated_error < nae[nstate]) {

					// save error for surviving state
					nae[nstate] = accumulated_error;

					// update state history
					state_history[nstate][t + 1] = state;
				}
			}
		}
		
		// get accumulated error ready for next time slice
		for(i = 0; i < (1 << (K - 1)); i++) {
			ae[i] = nae[i];
			nae[i] = max_error;
		}
	}

	// the final state is the state with the fewest errors
	min_state = (unsigned int)-1;
	min_error = max_error;
	for(i = 0; i < (1 << (K - 1)); i++) {
		if(ae[i] < min_error) {
			min_state = i;
			min_error = ae[i];
		}
	}

	// trace the path
	cur_state = min_state;
	for(t = input_size; t >= 1; t--) {
		min_state = cur_state;
		cur_state = state_history[cur_state][t]; // get previous
		output[t - 1] = prev_next_state[cur_state][min_state];
	}

	// return the number of errors detected (hard-decision)
	return min_error;
}
