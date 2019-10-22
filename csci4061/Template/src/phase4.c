#include "phase4.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void phase4(){
    int fd,fp,d2;
    char buff[128];
    fd = open("./FinalResult.txt", O_WRONLY);
    d2 = dup2(STDOUT_FILENO, fd);
    if((fp = fopen("./ReducerResult.txt", "r"))==NULL){
        perror("Failed to open the file from mapArray");
    }
    while(fgets(buff,128,fp)){
        printf("%s",buff);
    }



}