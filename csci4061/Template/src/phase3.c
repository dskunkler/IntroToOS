#include "phase3.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>



void phase3(int m,int mPipes[]){

  printf("HELLO FROM PHASE 3!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

	int charCount[26] = {0};
  int finalCount[26] = {0};
  int i,j,nbytes;


    for(i = 0; i < m; i++){
        close(mPipes[(2*i)+1]);
        //for(j = 0; j < 26; j++){
        nbytes = read(mPipes[(2*i)], charCount, sizeof(charCount));
        if(nbytes>0){
        //  printf("Bytes read = %d\n",nbytes);
          //printf("Hello from phase3 charCount[0] = %d\n",charCount[0]);
        }

        else{
          printf("READ FAILED\n");
        }
        for(j = 0; j < 26; j++){
          finalCount[j] += charCount[j];
        }
      // }
    }
    /*
    for(j = 0; j < 26; j++){
          printf("char %c = %d\n", j+'A', finalCount[j]);
    }*/

    FILE *RRfd;
    RRfd = fopen("ReducerResult.txt", "w");
    for(i = 0; i < 26; i++){
      fprintf(RRfd, "%c %d\n", 'A'+ i, finalCount[i]);
    }
    fclose(RRfd);








}
