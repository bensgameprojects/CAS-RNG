/*
Copyright 2016 Karolos Antoniadis, Peva Blanchard, Rachid Guerraoui and Julien Stainer <first.last@epfl.ch>

This file is part of CoRNG.

CoRNG is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CoRNG is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CoRNG.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __BLUM_ELIAS
#define __BLUM_ELIAS

#include <gmp.h>
//#include "bignum.h"

typedef struct _blumelias_instance {

	/*
	 * Parameters
	 */

	unsigned nstates; // states = 0, 1, .., nstates-1
	unsigned window;  // blum-elias window length
	unsigned seglen; // segment length in the trace
	unsigned eff_nstates; // effective nb. of states
	unsigned *weight; // precomputed weights for hashing

	/*
	 * Transducer state
	 */

	unsigned **exit;  // exit sequences
	unsigned long *len;  // length of the exit sequences

	/*
	 * Auxiliary variables
	 */

	unsigned *blockid;
	mpz_t binom; 
	mpz_t r; 
	mpz_t binom2;
	mpz_t r2;
	mpz_t blocksize;
	mpz_t mask;
	mpz_t base;
	mpz_t offset;
	mpz_t a;
	mpz_t a1;
	mpz_t b;
	mpz_t b1;
} be_instance;

void be_init(be_instance *X, unsigned nstates, unsigned window, unsigned seglen);
void be_close(be_instance *X);

void be_zero(be_instance *X);
void be_copy(be_instance *dst, be_instance *src); // both must have the same parameters
void be_traverse(be_instance *X, char trace[], unsigned long tracelen, unsigned long *consumed);

void be_rank2(be_instance *X, mpz_t ret, unsigned s0, unsigned *word, unsigned wordlen);
void be_rank(be_instance *X, mpz_t ret, unsigned nsym, unsigned *word, unsigned wordlen);
void be_elias(be_instance *X, unsigned seq[], unsigned seqlen, 
				char *out, unsigned *outlen);
void be_blumelias(be_instance *X, char trace[], unsigned long tracelen, unsigned long *consumed, 
				char *out, unsigned long *outlen);

void be_debug_print(be_instance *X);

#endif
