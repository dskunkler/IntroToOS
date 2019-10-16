#include "phase1.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
// You are free to use your own logic. The following points are just for getting started
/* 	Data Partitioning Phase - Only Master process involved
	1) 	Create 'MapperInput' folder

	2) 	Traverse the 'Sample' folder hierarchy and insert the text file paths
		to Mapper_i.txt in a load balanced manner
	3) 	Ensure to keep track of the number of text files for empty folder condition

*/
//returns the number of files in the Sample file

//Recursively moves through the path appending the text files to our array
int getNfiles(char* path, char mArray[][28], int i, int n){
	FILE *myfd;
    DIR *dr = opendir(path);
    struct stat statbuf;
    struct dirent *de;
    if(dr == NULL){
        printf("Invalid filepath = %s.\n", path);
    }
    if (dr != NULL){
		//if the directory exists continue
        while ((de = readdir(dr)) != NULL){
            if(strcmp(de->d_name,".")!= 0 && strcmp(de->d_name, "..")!= 0){
				//if its a directory, open it and recurse, tracking number of files appended to keep appropriate index
                if (de->d_type == DT_DIR){
                    char *newpath = '\0';
                    newpath = malloc(sizeof(path) + sizeof("/") + sizeof(de->d_name));
                    strcat(newpath, path);
                    strcat(newpath, "/");
                    strcat(newpath, de->d_name);
                    //printf("newpath = %s\n", newpath);
                    i += getNfiles(newpath, mArray, i,n);
                }
                else{
                    char *newpath = '\0';
                    newpath = malloc(sizeof(path) + sizeof("/") + sizeof(de->d_name));
                    strcat(newpath,path);
                    strcat(newpath, "/");
                    strcat(newpath,de->d_name);
					myfd = fopen(mArray[i],"a");
					fprintf(myfd,"%s\n",newpath);
                    i = (i+1)%n;
                }            
            }         
        }
    }
	return i;
}


void phase1(char* filepath, int m, char mapArray[][28]){

	pid_t cpid;
	int i;
	char numbuf[2];
	struct stat statbuf;


	//open the Sample Directory
	DIR *dr = opendir(filepath);
	if(dr == NULL){
        printf("Invalid filepath.\n");
		return;
    }
	//get the file status
	if(stat(filepath , &statbuf) == -1){
        printf("Failed to get file status\n");
		return;
    }
	// Check file status. If Sample folder contains only "." and ".." it's empty
	if((long)statbuf.st_size <= 2){
		printf("The Sample folder is empty\n");
		return;
	}

	//forks to execute a mkdir and create MapperInput directory
	cpid = fork();
	if (cpid == -1) {
        perror("fork failed");
    }
	else if(cpid > 0){
		wait(NULL);
	}
	else{
		char *makeDir[] = {"mkdir", "MapperInput", NULL};
		execvp(makeDir[0],makeDir);
	}

	//executes a for loops to initialize an array of filenames to be written to.
	for(i = 0; i < m; i++){
		//creates the proper filepath relative to current directory
		memset(mapArray[i],0, 28*sizeof(char));
		sprintf(numbuf,"%d", i);
		strcat(mapArray[i], "./MapperInput/Mapper_");
		strcat(mapArray[i],numbuf);
		strcat(mapArray[i], ".txt");		
	}
	getNfiles(filepath,mapArray,0, m);



}
