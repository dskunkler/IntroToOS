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
	int charCount[26] = {0};
  int finalCount[26] = {0};
  int i,j,nbytes;
  FILE *RRfd;

  for(i = 0; i < m; i++){
      close(mPipes[(2*i)+1]);
      //for(j = 0; j < 26; j++){
      nbytes = read(mPipes[(2*i)], charCount, sizeof(charCount));
      if(nbytes==0){
        printf("ERROR NO BYTES READ!\n");
      }
      for(j = 0; j < 26; j++){
        finalCount[j] += charCount[j];
      }
  }

  if(!(RRfd = fopen("ReducerResult.txt", "w"))){
    perror("REDUCERRESULT.TXT OPEN FAILED IN phase3\n");
  }
  for(i = 0; i < 26; i++){
    fprintf(RRfd, "%c %d\n", 'A'+ i, finalCount[i]);
  }
  fclose(RRfd);
}
