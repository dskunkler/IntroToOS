#include "phase4.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void phase4(){

  printf("Hello from phase 4!&&&&&&&&&&&&&\n");
  FILE *fp, *fp1;
  int fd, d2;
  char buff[128] = "Buff is empty";
  if((fp1 = fopen("./FinalResult.txt", "w")) == NULL){
      perror("Failed to open FinalResult.txt\n");
  }
  if((fd = fileno(fp1)) == -1){
    perror("fileno broken\n");
  }

  printf("fd = %d\n",fd);
  d2 = dup2(fd,STDOUT_FILENO);
  if((fp = fopen("./ReducerResult.txt", "r")) == NULL){
    perror("Failed to open the file from mapArray");
  }
  else{
    while(fgets(buff,128,fp)!=NULL){
      printf("%s",buff);
    }
  }



}
