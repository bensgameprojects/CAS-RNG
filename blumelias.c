
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//#include "bignum.h"
#include <gmp.h>

#include "blumelias.h"

void be_init(be_instance *X, unsigned nstates, unsigned window, unsigned seglen) {

	/*
	 * Blum-Elias parameter struct
	 */

	X->nstates = nstates;
	X->window = window;
	X->seglen = seglen;

	X->eff_nstates = 1; // eff_nstates = nstates^seglen
	for (unsigned i=0; i<seglen; i++)
		X->eff_nstates *= nstates;

	X->blockid = (unsigned *) malloc(X->eff_nstates*sizeof(unsigned));

	X->weight = (unsigned *) malloc(seglen*sizeof(unsigned));
	for (unsigned i=0; i<seglen; i++) {
		// weight[i] : nstates to the power of i
		X->weight[i] = 1;
		for (unsigned j=0; j<i; j++)
			X->weight[i] *= nstates;
	}

	X->exit = (unsigned **) malloc(X->eff_nstates * sizeof(unsigned *));
	for (unsigned s=0; s< X->eff_nstates; s++)
		X->exit[s] = (unsigned *) malloc(2*window * sizeof(unsigned));
	X->len = (unsigned long *) malloc(X->eff_nstates * sizeof(unsigned long));
	for (unsigned s=0; s< X->eff_nstates; s++)
		X->len[s] = 0;

	/*
	 * GMP integers init
	 */

	mpz_init2(X->binom, 128);
	mpz_init2(X->binom2, 128);
	mpz_init2(X->r2, 128);

	mpz_init2(X->blocksize, 128);
	mpz_init2(X->mask, 128);
	mpz_init2(X->base, 128);
	mpz_init2(X->offset, 128);
	mpz_init2(X->r, 128);

	mpz_init2(X->a, 128);
	mpz_init2(X->a1, 128);
	mpz_init2(X->b, 128);
	mpz_init2(X->b1, 128);
}

void be_close(be_instance *X) {
	if (X->weight) free(X->weight);
	if (X->blockid) free(X->blockid);
	for (unsigned i=0; i<X->eff_nstates; i++)
		if (X->exit[i]) free(X->exit[i]);
	if (X->exit) free(X->exit);
	if (X->len) free(X->len);

	mpz_clear(X->binom);
	mpz_clear(X->r);
	mpz_clear(X->binom2);
	mpz_clear(X->r2);
	mpz_clear(X->blocksize);
	mpz_clear(X->mask);
	mpz_clear(X->base);
	mpz_clear(X->offset);
	mpz_clear(X->a);
	mpz_clear(X->a1);
	mpz_clear(X->b);
	mpz_clear(X->b1);
}

void be_zero(be_instance *X) {
	for (unsigned s=0; s < X->eff_nstates; s++)
		X->len[s] = 0;
}

void be_copy(be_instance *dst, be_instance *src) {
	for (unsigned s=0; s<dst->eff_nstates; s++) {
		dst->len[s] = src->len[s];
		for (unsigned long i=0; i<dst->len[s]; i++)
			dst->exit[s][i] = src->exit[s][i];
	}
}

void be_traverse(be_instance *X, char trace[], unsigned long tracelen, unsigned long *consumed) {
	unsigned long cur;
	for (cur=0; cur < tracelen - X->seglen; cur++) {
		unsigned long cur_s = 0;
		unsigned long nex_s = 0;
		for (unsigned long i=cur; i < cur + X->seglen; i++) {
			cur_s += ((unsigned long)trace[i]) * X->weight[i-cur];
			nex_s += ((unsigned long)trace[i+1]) * X->weight[i-cur];
		}
		if (X->len[nex_s] >= X->window)
			X->len[nex_s] = 0;
		X->exit[cur_s][X->len[cur_s]] = nex_s;
		X->len[cur_s]++;
	}
	*consumed = tracelen - X->seglen;
}

void be_rank2(be_instance *X, mpz_t ret, unsigned s0, unsigned *word, unsigned wordlen) {
	unsigned pos = wordlen - 1;
	unsigned count0 = 0;
	unsigned count1 = 0;
	unsigned realpos;
	unsigned left;


	//fprintf(stderr,"be_rank2");
	mpz_set_si(ret, 0);

	for (unsigned i=0; i<wordlen; i++){
		if (word[i] == s0) count0++;
		if (word[i] > s0) count1++;
	}
	left = count1;
	realpos = count0 + count1 - 1;
	while (left > 0) {
		while (word[pos] <= s0) {
			if (word[pos] == s0) {
				//fprintf(stderr,"(%d, %d)\n", realpos, left - 1);
				mpz_bin_uiui(X->binom2, realpos, left-1); // choose left-1 among realpos
				mpz_add(ret, ret, X->binom2); //ret += binomial(left-1,realpos);
				realpos--;
			}
			pos--;
		}
		left--;
		pos--;
		realpos--;
	}
}

void be_rank(be_instance *X, mpz_t ret, unsigned nsym, unsigned *word, unsigned wordlen) {
	unsigned smin = 0;
	unsigned c;
	unsigned len;
	//fprintf(stderr,"be_rank");

	mpz_set_ui(X->a, 1); //a = 1;
	mpz_set_ui(X->b, 0); //b = 0;
	while (smin < nsym-2) {
		c = 0;
		len = 0;

		for (unsigned i=0; i<wordlen; i++) {
			if (word[i] == smin) c++;
			if (word[i] >= smin) len++;
		}
		
		mpz_bin_uiui(X->binom, len, c); //choose c among len
		mpz_mul(X->a1, X->a, X->binom); // a1 = a*binom;

		be_rank2(X, X->r2, smin, word, wordlen); // r2 = rank2(...)

		mpz_mul(X->b1, X->r2, X->a);  // b1 = r2*a + b;
		mpz_add(X->b1, X->b1, X->b);

		mpz_set(X->a, X->a1); // a = a1;
		mpz_set(X->b, X->b1); // b = b1;

		smin++;
	}

	be_rank2(X, X->r2, smin, word, wordlen);

	// ret = a*r2 + b
	mpz_mul(ret, X->a, X->r2);
	mpz_add(ret, ret, X->b);
}

void be_elias(be_instance *X, unsigned seq[], unsigned seqlen, 
		char *out, unsigned *outlen) {
	unsigned offsetlen;

	//fprintf(stderr,"be_elias\n");

	// count the occurrences of each symbol in seq
	for (unsigned s=0; s < X->eff_nstates; s++) X->blockid[s] = 0;
	for (unsigned i=0; i<seqlen; i++) {
		if (seq[i] < X->eff_nstates) X->blockid[seq[i]]++;
		else fprintf(stderr, "error: state %d >= eff_nstates = %d", seq[i], X->eff_nstates); // error
	}


	// compute multinomial coeff
	// very stupid method (iterated prod of binomials)
	unsigned w = seqlen;
	mpz_set_ui(X->blocksize, 1);
	for (unsigned s=0; s<X->eff_nstates; s++) {
		mpz_bin_uiui(X->binom, w, X->blockid[s]);
		mpz_mul(X->blocksize, X->blocksize, X->binom);
		w -= X->blockid[s]; 
		//fprintf(stderr,"be_elias for\n");
	}	


	// compute rank
	be_rank(X, X->r, X->eff_nstates, seq, seqlen);
	

	if (mpz_cmp(X->r, X->blocksize)>0) {
		gmp_fprintf(stderr, "--- OVERFLOW(elias) ---\n");
		gmp_fprintf(stderr, "seq : ");
		for (unsigned i=0; i<seqlen; i++)
			gmp_fprintf(stderr, "%d ", seq[i]);
		gmp_fprintf(stderr, "\n");
		gmp_fprintf(stderr, "blockid : ");
		for (unsigned s=0; s<X->eff_nstates; s++)
			gmp_fprintf(stderr, "%d ", X->blockid[s]);
		gmp_fprintf(stderr, "\n");
		gmp_fprintf(stderr, "rank : %Zd\n", X->r);
		gmp_fprintf(stderr, "blocksize : %Zd\n", X->blocksize);

		//fprintf(stderr,"be-elias before sleeping for 2\n");
		sleep(2);
		return;
	}


	// compute base, mask, offset len
	mpz_set_ui(X->mask, 0); //mask = 0;
	mpz_set_ui(X->base, 0); //base = 0;

	mpz_and(X->a, X->mask, X->blocksize);
	//offsetlen = 0;


	while (mpz_cmp(X->a, X->r) <= 0) { // mask & blocksize <= rank
		//mask = (mask << 1) + 1;
		mpz_mul_2exp(X->mask, X->mask, 1);
		mpz_add_ui(X->mask, X->mask, 1);

		//offsetlen++;
		
		// X->a = mask & blocksize
		mpz_and(X->a, X->mask, X->blocksize);
		//fprintf(stderr,"be_elias while\nX-a=");
		//print_num(X->a);
		//fprintf(stderr,"\nX->blocksize=");
		//print_num(X->blocksize);
		//fprintf(stderr,"\nX->r=");
		//print_num(X->r);
		//fprintf(stderr,"\n");



		if (mpz_cmp(X->a, X->r) <= 0 ) { // a <= rank
			mpz_set(X->base, X->a);
		}

	}
	// a is the smallest prefix of blocksize > rank

	mpz_sub(X->offset, X->r, X->base); //offset = rank - base;

	mpz_sub(X->a1, X->a, X->base); // should be 2 to the power of offsetlen
	
	
	offsetlen = mpz_sizeinbase(X->a1, 2) - 1;

	*outlen = offsetlen;

	//gmp_fprintf(stderr, "bsize %Zd\t rank %Zd\t base %Zd\t offset %Zd\t offsetlen %d\n",X->blocksize,X->r,X->base,X->offset, offsetlen);


	// write offset's binary expansion
	
	for (unsigned i=0; i<offsetlen; i++) {
		//fprintf(stderr,"be_elias second for\n");

		// b = offset >> i
		mpz_tdiv_q_2exp(X->b, X->offset, i);

		// test the first bit of b
		out[offsetlen-1-i] = mpz_tstbit(X->b,0);

	}

}

void be_blumelias(be_instance *X, char trace[], unsigned long tracelen, 
	       unsigned long *consumed, char *out, unsigned long *outlen) {
	unsigned long cur;
	*outlen = 0;

	//fprintf(stderr,"be_blumelias");
	for (cur=0; cur < tracelen - X->seglen; cur++) {

		// hash the current and next segment
		unsigned long cur_s = 0;
		unsigned long nex_s = 0;
		for (unsigned long i=cur; i < cur + X->seglen; i++) {
			cur_s += ((unsigned long)trace[i]) * X->weight[i-cur];
			nex_s += ((unsigned long)trace[i+1]) * X->weight[i-cur];
			//fprintf(stderr,"be_blumelias inside for for ...");
		}
		//fprintf(stderr,"be_blumelias inside for  ...");

		// blum part
		// exit sequence of the next state is long enough
		if (X->len[nex_s] >= X->window) { 
			unsigned tmp;


			// elias part
			be_elias(X, X->exit[nex_s], X->len[nex_s], &out[*outlen], &tmp);
			*outlen += tmp;
			X->len[nex_s] = 0;
		}

		X->exit[cur_s][X->len[cur_s]] = nex_s; 	// append state to current 
							// exit sequence
		X->len[cur_s]++;
	}

	*consumed = tracelen - X->seglen;
}

void be_debug_print(be_instance *X) {
	fprintf(stderr, "------- DEBUG PRINT INSTANCE ------\n");
	if (!X) {
		fprintf(stderr, "null instance\n");
		return;
	}
	fprintf(stderr, "nstates\t%d\n",X->nstates);
	fprintf(stderr, "window\t%d\n",X->window);
	fprintf(stderr, "seglen\t%d\n",X->seglen);
	fprintf(stderr, "eff nstates\t%d\n",X->eff_nstates);
	fprintf(stderr, "weight\t");
	if (X->weight)
		for (int i=0; i<X->seglen; i++) fprintf(stderr,"%d ",X->weight[i]);
	else
		fprintf(stderr,"null");
	fprintf(stderr, "\n");
/*
	fprintf(stderr,"exit sequences\n");
	if (!X->exit) fprintf(stderr,"\tnull\n");
	else {
		for (int i=0; i<X->eff_nstates; i++) {
			if (X->len[i] > 0) {
				fprintf(stderr,"\texit[%d] (len %d) : ",i,X->len[i]);
				for (int j=0; j<X->len[i]; j++)
					fprintf(stderr, "%d ", X->exit[i][j]);
				fprintf(stderr,"\n");
			}
		}
	}
	fprintf(stderr,"blockid\t");
	if (X->blockid)
		for (int i=0; i<X->eff_nstates; i++)
			fprintf(stderr, "%d ",X->blockid[i]);
	else fprintf(stderr,"null");
	fprintf(stderr,"\n");
*/
/*
	gmp_fprintf(stderr,"binom\t%Zd\n",X->binom);
	gmp_fprintf(stderr,"r\t%Zd\n",X->r);
	gmp_fprintf(stderr,"binom2\t%Zd\n",X->binom2);
	gmp_fprintf(stderr,"r2\t%Zd\n",X->r2);
	gmp_fprintf(stderr,"blocksize\t%Zd\n",X->blocksize);
	gmp_fprintf(stderr,"mask\t%Zd\n",X->mask);
	gmp_fprintf(stderr,"base\t%Zd\n",X->base);
	gmp_fprintf(stderr,"offset\t%Zd\n",X->offset);
	gmp_fprintf(stderr,"a\t%Zd\n",X->a);
	gmp_fprintf(stderr,"a1\t%Zd\n",X->a1);
	gmp_fprintf(stderr,"b\t%Zd\n",X->b);
	gmp_fprintf(stderr,"b1\t%Zd\n",X->b1);
	*/
}
