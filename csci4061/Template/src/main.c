#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]){

	// argument count check
	if(argc != 3){
		printf("Incorrect useage. Should be ./exname /path numprocesses");
	}
	int catch = 0;
	int i = 0;
	int m = atoi(argv[2]);
	char mapArray[m][128];
	int mPipes[m+m];


	catch = phase1(argv[1],m, mapArray);
	if(catch == -1){
		printf("Error in phase1. Goodbye\n");
		return 1;
	}

	//create m pipes
	for(i = 0; i < m; i++){
		//printf("Hello from pipes\n");
		if(pipe(mPipes+(2*i)) == -1){
			perror("Failed to create the pipe");
		}
	}
	
	catch = phase2(argv[1],m, mapArray, mPipes);
	if(catch == 0){
		return 1;
	}
	phase3(m,mPipes);
	




	//just make a function call to code in phase1.c
	//phase1 - Data Partition Phase

	//create pipes

	//just make a function call to code in phase2.c
	//phase2 - Map Function

	//just make a function call to code in phase3.c
	//phase3 - Reduce Function

	//phase4
	//wait for all processes to reach this point

	//just make a function call to code in phase4.c
	//master process reports the final output

	return 0;

}
