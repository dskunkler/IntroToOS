#include "phase2.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
// You are free to use your own logic. The following points are just for getting started
/* 	Map Function
	1)	Each mapper selects a Mapper_i.txt to work with
	2)	Creates a list of letter, wordcount from the text files found in the Mapper_i.txt
	3)	Send the list to Reducer via pipes with proper closing of ends
*/

//used to clean up line later.
int makeargv(const char *s, const char *delimiters, char ***argvp)
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

int phase2(char *filepath,int m,char mapArray[][128],int mPipes[]){
	int ma;
	char delim[] = " \n";
	char **myargv;
	pid_t c_pid;
	int i, j;
	int charCount[26] = {0};
	char mapperLine[128],textLine[128];
	FILE* fp, *fp1;

//create m mapper processes which go through their particular folder and generate number of letters
	for(i = 0; i < m; i++){
		//printf("i = %d\n",i);
		c_pid = fork();

		if(c_pid < 0){
			printf("Error forking\n");
		}
		if(c_pid >0 ){
			wait(NULL);
		}
		
   //child process
		if(c_pid == 0){
			//closes all read ends of pipes and all writes accept for its own.
			for(j = 0; j< m; j++){
				if(i!= j){
					if(close(mPipes[2*j+1]) == -1){
						perror("ERROR WRITE ENDS IN CHILD PIPE");
					}
					if(close(mPipes[2*j]) == -1){
						perror("ERROR CLOSING OWN READ END IN CHILD");
					}
				}
				else{
					if(close(mPipes[2*j]) == -1){
						perror("ERROR CLOSING OTHER READ ENDS IN CHILD");
					}
				}
			}

			//open the mapperfile
			if((fp = fopen(mapArray[i], "r"))==NULL){
				perror("Failed to open the file from mapArray");
			}

			//get the target for our word count from mapArray
			while(fgets(mapperLine,128,fp)){

				//clean up the target
				ma = makeargv(mapperLine,delim,&myargv);

				//open target
				if((fp1 = fopen(myargv[0], "r+"))==NULL){
					perror("Failed mapperline open\n");
				}
				else{
					//get each line and increment the appropriate index corresponding to the letter
					while(fgets(textLine, 128, fp1)){
						charCount[toupper(textLine[0]) -'A'] += 1;
					}
				}
			}
			//Pipe the array to phase 3
			write(mPipes[(2*i)+1], charCount, sizeof(charCount));
			close(mPipes[2*i+1]);
			if(fclose(fp)!= 0){
				perror("ERROR CLOSING fp\n");
			}
			if(fclose(fp1)!=0){
				perror("ERROR CLOSING fp1\n");
			}
			return c_pid;
		}
	}
}
