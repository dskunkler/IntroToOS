#define _DEFAULT_SOURCE
#include "phase1.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
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

*/int makeargv(const char *s, const char *delimiters, char ***argvp)
{
  int error;
  int i;
  int numtokens;
  const char *snew;
  char *t;

  if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
    errno = EINVAL;
    return -1;
  }
  *argvp = NULL;
  snew = s + strspn(s, delimiters);         /* snew is real start of string */
	//printf("snew set = %s\n", snew);
  if ((t = malloc(strlen(snew) + 1)) == NULL){
		//printf("malloc of t failed \n");
    return -1;
	}

  strcpy(t, snew);
  numtokens = 0;
  if(strtok(t, delimiters) != NULL)     /* count the number of tokens in s */
    for(numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++){};
		/* create argument array for ptrs to the tokens */
	if((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL){
		error = errno;
		free(t);
		errno = error;
		//printf("malloc of argvp failed\n");
		return -1;
	}
  /* insert pointers to tokens into the argument array */
  if (numtokens == 0)
    free(t);
  else {
    strcpy(t, snew);
    **argvp = strtok(t, delimiters);
    for (i = 1; i < numtokens; i++)
      *((*argvp) + i) = strtok(NULL, delimiters);
    }
	*((*argvp) + numtokens) = NULL;            /* put in final NULL pointer */
	//printf("returning numtokens = %d\n",numtokens);
	return numtokens;
}

//Recursively moves through the path appending the text files to our Mapper_i.txt files
int getNfiles(char* path, char mArray[][128], int i, int n){
  	FILE *myfd;
    DIR *dr = opendir(path);
	int ma;
	char delim[] = " \n";
	char **myargv;
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
                    char *newpath;
					ma = makeargv(de->d_name,delim, &myargv);
				//	printf("path = %s\nname = %s\n",path, myargv[0]);
                    newpath = malloc(sizeof(path) + sizeof("/") + sizeof(de->d_name));
					memset(newpath, '\0',sizeof(newpath));
                    strcat(newpath, path);
                    strcat(newpath, "/");
                    strcat(newpath, de->d_name);
					//printf("i before function return = %d\n",i);
                    i = (i + getNfiles(newpath, mArray, i,n)) % n;
					//printf("I AFTER RETURN FROM getNFILES  = %d\n", i);
					free(newpath);
                }
				//if not a directory, append the path and txt file name into Mapper_i.txt.
                else{
                    char *newpath;
                    newpath = malloc(sizeof(path) + sizeof("/") + sizeof(de->d_name));
					memset(newpath, '\0',sizeof(newpath));
                    strcat(newpath,path);
                    strcat(newpath, "/");
					strcat(newpath,de->d_name);
					//printf("i TO CRASH IT= %d\n",i);
					//printf("path = %s\n", path);
					//printf("newpath = %s\n", newpath);
					myfd = fopen(mArray[i],"a+");
					fprintf(myfd,"%s\n",newpath);
					fclose(myfd);
					free(newpath);
                    i = (i+1)%n;
					//printf("new i = %d\n",i);
                }
            }
        }
    }
	free(dr);
	return i;
}


int phase1(char* filepath, int m, char mapArray[][128]){
	printf("******************************HELLO FROM PHASE 1\n\n");

	pid_t cpid;
	int i;
	char numbuf[2];
	struct stat statbuf;


	//open the Sample Directory
	DIR *dr = opendir(filepath);
	if(dr == NULL){
        printf("Invalid filepath.\n");
		return -1;
    }
	free(dr);
	//get the file status
	if(stat(filepath , &statbuf) == -1){
        printf("Failed to get file status\n");
		return -1;
    }
	// Check file status. If Sample folder contains only "." and ".." it's empty
	if((long)statbuf.st_size <= 2){
		printf("The Sample folder is empty\n");
		return -1;
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
	//populates Mapper_i.txt files with text files from target path
	getNfiles(filepath,mapArray,0, m);



		printf("******************************GOODBYE FROM PHASE 1\n\n");
}
