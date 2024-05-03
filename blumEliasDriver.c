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

#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<linux/random.h>
#include<poll.h>
#include "blumelias.h"

#define WINDOW 20
				    
//#define NROUNDS 4194304
		   
#define FILTERRATIO 10

#define OUT_POOL 0
#define OUT_STDOUT 1



int main(int argc, char **argv){
	char mode;
	long nbytes; // number of bytes requested
        uint32_t NSTATES = 0;
	uint32_t NROUNDS = 0;
	uint32_t NBINSTANCES = 0;
	uint32_t SEGLEN = 1;
	int appendFlag  = 0;
	int printFlag = 0;
 	char* fileName = "randomBits.bin";
	if (argc == 7) {
	  mode = OUT_STDOUT;//OUT_POOL
		NSTATES = atoi(argv[2]);
		NROUNDS = atoi(argv[1]);
		NBINSTANCES = atoi(argv[3]);
		appendFlag = atoi(argv[4]);
		printFlag = atoi(argv[5]);
	        fileName = argv[6];
	}/* else if (argc == 5) {
		mode = OUT_STDOUT;
		NSTATES = atoi(argv[1]);
		NBINSTANCES = atoi(argv[2]);
		SEGLEN = atoi(argv[3]);
		nbytes = atol(argv[4]);
		
		fprintf(stderr, "%d %d and %lld\n", NBINSTANCES, SEGLEN, nbytes);
		if (nbytes <= 0) {
			fprintf(stderr, "error: negative requested number of bytes.\n");
			exit(EXIT_FAILURE);
		}
		}*/
	else {
		fprintf(stderr, "Usage: ./blumelias [NROUNDS] [NSTATES] [NBINSTANCES]\n");
		exit(EXIT_FAILURE);
	}
	
	/*	char *readtrace=malloc(NROUNDS*sizeof(char));
	if(readtrace==NULL){
		perror("Allocating memory failed.");
		exit(EXIT_FAILURE);
		}*/

	FILE *coobsfd=fopen("trace.bin","r");
	if(coobsfd==NULL){
		perror("Opening trace.txt failed.");
		exit(EXIT_FAILURE);
	}

	unsigned long rawtracelen=NSTATES*NROUNDS;
	char *rawtrace=malloc(rawtracelen*sizeof(char));
	if(rawtrace==NULL){
		perror("Allocating memory failed.");
		exit(EXIT_FAILURE);
	}

	struct {
		int entropy_count;
		int buf_size;
		__u32 buf[256];
	} entropy;

	int randfd;
	struct pollfd fds[1];
	/*
	if (mode == OUT_POOL) {
		randfd=open("/dev/random", O_WRONLY);
		if(randfd<0){
			perror("Opening /dev/random failed.");
			exit(EXIT_FAILURE);
		}
	}
	*/
	fds[0].fd=randfd;
	fds[0].events=POLLOUT;
	entropy.entropy_count=7*256;
	entropy.buf_size=256;

	char *finalbuf;
	unsigned long totlen;

	be_instance X; // dummy instance for traversal
	be_instance *Y = (be_instance *) malloc(NBINSTANCES * sizeof(be_instance));
	if (Y == NULL) {
		fprintf(stderr, "Couldn't allocate memory: Y = malloc(%d * sizeof(be_instance))", NBINSTANCES);
		exit(EXIT_FAILURE);
	}
	// init
	be_init(&X, NSTATES, WINDOW, SEGLEN);
	for (unsigned i=0; i<NBINSTANCES; i++)
		be_init(&Y[i], NSTATES, WINDOW, SEGLEN);

	long nbytes_count = 0;
	size_t readbytes=fread(rawtrace, 1, NSTATES*NROUNDS, coobsfd);
	while ( readbytes == NSTATES*NROUNDS ){
	  //printf("readbytes matched expected...\n");
	  if(printFlag){
	    for(int i = 0; i < readbytes; i++){
	      //rawtrace[i] = rawtrace[i] - '0';
	      printf("%d, ", rawtrace[i]);
	    }
	    printf("\n");
	  }
	  //char pbeg = rawtrace[0];
	  int64_t beg = 0;
		/*	if (pbeg != 4) { // not w0 || w1
			beg=1;
			while (beg < rawtracelen && rawtrace[beg]==pbeg) beg++;
		} else beg=0;
		*/
	  //char pend = rawtrace[rawtracelen-1];
	  int64_t end = rawtracelen-1;
		/*
		if (pend != 4) { // not w0 || w1
			end = rawtracelen-2;
			while (end > 0 && rawtrace[end]==pend) end--;
		} else end = rawtracelen-1;
		*/
	  int64_t tracelen = end-beg+1;
	  if (tracelen <= 0) {
	    fprintf(stderr, "Error: non-positive (cleaned) tracelen %ld",tracelen);
	    //exit(EXIT_FAILURE);
	    continue;
	  }
	  char *trace = &rawtrace[beg];
	  //printf("beg: %d\n", beg);
	  unsigned long *chunkpos = (unsigned long *) malloc(NBINSTANCES * sizeof(unsigned long));
	  if (chunkpos == NULL) {
	    fprintf(stderr, "Couldn't allocate memory: chunkpos = malloc(%d * sizeof(unsigned long))", NBINSTANCES);
	    exit(EXIT_FAILURE);
	  }
	  unsigned long *chunklen = (unsigned long *) malloc(NBINSTANCES * sizeof(unsigned long));
	  if (chunklen == NULL) {
	    fprintf(stderr, "Couldn't allocate memory: chunklen = malloc(%d * sizeof(unsigned long))", NBINSTANCES);
	    exit(EXIT_FAILURE);
	  }
	  
	  memset((void*) chunklen, 0, NBINSTANCES*sizeof(unsigned long));
	  memset((void*) chunkpos, 0, NBINSTANCES*sizeof(unsigned long));
	
	  // init
	  be_zero(&X);
	  for (unsigned i=0; i<NBINSTANCES; i++)
	    be_zero(&Y[i]);
	  //printf("About to traverse...\n");
	  // traverse
	  unsigned long z = 0;
	  unsigned long c = 0;
	  for (unsigned i=0; i<NBINSTANCES; i++) {
	    
	    be_copy(&Y[i], &X); // copy exit sequences
	    //be_debug_print(&Y[i]);
	    chunkpos[i] = z;
	    if (i < NBINSTANCES-1) {
	      //printf("Traversing %d\n", i);
	      be_traverse(&X, &trace[z], tracelen/NBINSTANCES, &c);
	      //printf("Traversed %d\n", i);
	      chunklen[i] = c;
	    } else {
	      be_traverse(&X, &trace[z], tracelen - z, &c);
	      chunklen[i] = c;
	    }
	    z += c;
	  }
	  
	  // work
	  char **out = (char **) malloc(NBINSTANCES * sizeof(char*));
	  if (out == NULL) {
	    fprintf(stderr, "Couldn't allocate memory: out = malloc(%d * sizeof(char*))", NBINSTANCES);
	    exit(EXIT_FAILURE);
	  }
	  for (unsigned i=0; i<NBINSTANCES; i++) {
	    out[i] = (char *) malloc(4*chunklen[i]*sizeof(char));
	    if (out[i] == NULL) {
	      fprintf(stderr, "Couldn't allocate memory: out[%d] = malloc(4*%ld*sizeof(char))\n", i, chunklen[i]);
	      exit(EXIT_FAILURE);
	    }
	  }
	  
	  unsigned long *outlen = (unsigned long *) malloc(NBINSTANCES * sizeof(unsigned long));
	  if (outlen == NULL) {
	    fprintf(stderr, "Couldn't allocate memory: outlen = malloc(%d*sizeof(unsigned long))\n", NBINSTANCES);
	    exit(EXIT_FAILURE);
	  }
	  memset((void*) outlen, 0, NBINSTANCES*sizeof(unsigned long));
	  
	  // open mp here
	  ///*
	  //#pragma omp parallel for
	  for (unsigned i=0; i<NBINSTANCES; i++) {
	    be_blumelias(&Y[i], &trace[chunkpos[i]], chunklen[i], &c, 
			 out[i], &outlen[i]);
	  }
	  //*/
	  
	  
	  // concatenate successive output
	  ///*	
	  totlen=0; // total output length
	  for (unsigned k=0; k<NBINSTANCES; k++)
	    totlen += outlen[k];
	  char *totout = (char *) malloc(totlen*sizeof(char)); // total output
	  if (totout == NULL) {
	    fprintf(stderr, "Couldn't allocate memory: totout = malloc(%ld*sizeof(char))\n", totlen);
	    exit(EXIT_FAILURE);
	  }
	  memset((void*) totout, 0, totlen*sizeof(char));
	  
	  unsigned long idx=0;
	  for (unsigned k=0; k<NBINSTANCES; k++) {
	    for (unsigned long i=0; i<outlen[k]; i++) {
	      totout[idx] = out[k][i];
	      idx++;
	    }
	  }
	  if(printFlag){
	    printf("Print Binary Output from blum-elias:\n");
	    for(unsigned k = 0; k< totlen; k++){
	      printf("%d, ", totout[k]);
	    }
	    printf("\n");
	  }
	  FILE *randout;
	  if(appendFlag){
	    randout = fopen(fileName, "ab+");
	  }
	  else{
	    randout = fopen(fileName, "wb+");
	  }
	   
	  fwrite(totout, 1, totlen, randout);
	  fclose(randout);
	  free(outlen);
	  for (unsigned j=0; j<NBINSTANCES; j++)
	    free(out[j]);
	  free(out);
	  free(chunklen);
	  free(chunkpos);
	  
	  unsigned char b;
	  finalbuf=(char*) malloc((totlen/8)*sizeof(char));
	  if(finalbuf==NULL){
	    perror("Memory allocation failed.");
	    exit(EXIT_FAILURE);
	  }
	  
	  for (unsigned long i=0; 8*i+7 < totlen; i++) {
	    b = 0;
	    for (unsigned j=0; j<8; j++)
	      b += totout[8*i + 7-j] << j;
	    finalbuf[i]=b;
	  }
	  totlen=totlen/8;
	  free(totout);
	  if(printFlag){
	    for(unsigned k = 0; k < totlen; k++){
	      printf("%u, ", (unsigned char)finalbuf[k]);
	    }
	    printf("\n");
	  }
	  if(totlen < tracelen/(FILTERRATIO*2)){
	    fprintf(stderr, "Output too small, experience discarded. Check configuration and/or machine charge.\n");
	  }
	  else{
	    fprintf(stderr, "Output ok: %lu bytes ready to be consumed.\n",totlen);
	    //printf("Print Output chars from finalbuf merge of totout to file randomChars.bin:\n");
	    FILE* randChars;
	    if(appendFlag){
	      randChars = fopen("randomChars.bin", "ab+");
	    }
	    else{
	      randChars = fopen("randomChars.bin", "wb+");
	    }
	    fwrite(finalbuf, 1, totlen, randChars);
	    fclose(randChars);
	  }
	  free(finalbuf);
	  readbytes=fread(rawtrace, 1, NSTATES*NROUNDS, coobsfd);
	}
	/*
	if(readbytes == 0){
	  printf("Finished reading file... exiting...\n");
	}
	*/
	/*
        if(readbytes!=NSTATES*NROUNDS){
	  printf("readbytes: %d\n Expected: %d\n", readbytes,
		 NSTATES*NROUNDS);
	  perror("Reading trace.txt failed.");
	}
	*/
	// close
	be_close(&X);
	for (unsigned i=0; i<NBINSTANCES; i++)
	       	be_close(&Y[i]);
	free(Y);

	//free(readtrace);
	fclose(coobsfd);
	free(rawtrace);
	if(mode==OUT_POOL) close(randfd);

	exit(EXIT_SUCCESS);
}
