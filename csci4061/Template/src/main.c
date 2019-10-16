#include "main.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	// argument count check

	char mapArray[atoi(argv[2])][28];
	phase1(argv[1],atoi(argv[2]), mapArray);
	printf("%d\n", 'a'-'Z');

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
