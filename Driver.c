/*
 * This program drives the trace generator and the blum elias algorithm
 * to construct at least the number of bits specified as input
 */
#include<stdlib.h>
#include<stdint.h>
#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char** argv) {
  char mode;
  unsigned int lengthOfTrace = 0;
  int appendOutput = 0;
  int printOutput = 0;
  char* fileName = "randomBits.bin";
  if(argc >= 2){
    lengthOfTrace = atoi(argv[1]);
    if(lengthOfTrace <= 0){
      printf("Error: incorrect trace length(in 16millions). Must be >= 1 /n");
      return 0;
    }
  }
  if(argc == 3){
    if(argv[2][0] == '-' && argv[2][1] == 'a' && argv[2][2] == 0){
      appendOutput = 1;
    }
    else if(argv[2][0] == '-' && argv[2][1] == 'p' && argv[2][2] == 0){
      printOutput = 1;
    }
    else{
      fileName = argv[2];
    }
  }
  if(argc == 4){
    if(argv[2][0] == '-' && argv[2][1] == 'a' && argv[2][2] == 0
       || argv[3][0] == '-' && argv[3][1] == 'a' && argv[3][2] == 0){
      appendOutput = 1;
    }
    if(argv[2][0] == '-' && argv[2][1] == 'p' && argv[2][2] == 0
       || argv[3][0] == '-' && argv[3][1] == 'p' && argv[3][2] == 0){
      printOutput = 1;
    }
    if(printOutput == 0 || appendOutput == 0){
      fileName = argv[3];
    }
  }
  if(argc == 5){
    fileName = argv[4];
  }
  if(argc < 2 || argc > 5){
    printf("Usage: ./casrng [lengthOfTrace(in16Millions)] [-a to append output] [-p to print output] [fileName]\n");
    return 0;
  }

  //./casrng [lengthOfTrace(inMillions)] [-a to append output] [-p to print output]
  //argc == 4
  // make two process which run same 
  // program after this instruction

  for(int i = 0; i< lengthOfTrace; i++){
    
    printf("Starting Trace Generation %d...\n", i);
    pid_t pid = fork(); 
    if(pid == 0){
      //child process
      //Start generating trace
      char* args[] =  {"java", "-Djava.library.path=.", "Main",
		       "1000000", "16", "F", NULL};
      execvp(args[0], args);
    }
    else{
      //parent process
      //wait for trace to be generated:
      int waitstatus;
      waitpid(pid, &waitstatus, 0);
      printf("Trace Generation Finished!\n");
      printf("Starting Random Bits Extraction...\n");
      //run blum elias:
      pid_t blumElias = fork();
      if(blumElias == 0){
	char* argsb[] = {"./blumelias", "1000000", "16", "4",
			 "0", "0", fileName, NULL};
	if(appendOutput == 1){
	  argsb[4] = "1";
	}
	if(printOutput == 1){
	  argsb[5] = "1";
	}
	execvp(argsb[0], argsb);
      }
      else{
	waitpid(blumElias, &waitstatus, 0);
	printf("Random Bits Extraction Finished!\n");
	printf("Random Bits ready in file %s\n", fileName);
      }
    }
  }
  return 0; 
} 
