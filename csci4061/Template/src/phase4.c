#include "phase4.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void phase4(){
  FILE *fp, *fp1;
  int fd, d2;
  char buff[128] = "Buff is empty";

  //Open FinalResult.txt
  if((fp1 = fopen("./FinalResult.txt", "w")) == NULL){
      perror("Failed to open FinalResult.txt\n");
  }
  //get fileno from fp1
  if((fd = fileno(fp1)) == -1){
    perror("fileno broken\n");
  }
  //redirect stdoutput to the file descriptor of FinalResult.txt
  d2 = dup2(fd,STDOUT_FILENO);
  //open ReducerResult.txt
  if((fp = fopen("./ReducerResult.txt", "r")) == NULL){
    perror("Failed to open the file from mapArray");
  }
  //Write ReducerResult to FinalResult via stdoutput
  else{
    while(fgets(buff,128,fp)!=NULL){
      printf("%s",buff);
    }
  }
  if(fclose(fp1) == -1){
    perror("Close of fp1 failed in phase4\n");
  }
  if(fclose(fp) == -1){
    perror("Close of fp in phase4\n");
  }
}
