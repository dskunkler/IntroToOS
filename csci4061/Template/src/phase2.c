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

int makeargv(const char *s, const char *delimiters, char ***argvp);




int phase2(char *filepath,int m,char mapArray[][128],int mPipes[]){

	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^Hello from phase2\n");

	int ma;
	char delim[] = " \n";
	char **myargv;
	pid_t c_pid;
	int i, j;
	int fd;
	int charCount[26] = {0};
	char mapperLine[128],textLine[128], letCount[128] = "";
	FILE* fp, *fp1;

//close the read end of the pipe



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
						perror("ERROR CLOSING OTHET READ ENDS IN CHILD");
					}
				}
			}
			//printf("Hello from the child %d\n",i);
			//open the mapperfile
			if((fp = fopen(mapArray[i], "r"))==NULL){
				perror("Failed to open the file from mapArray");
			}

			//get the target for our word count from mapper
			while(fgets(mapperLine,128,fp)){

				//clean up the target
				ma = makeargv(mapperLine,delim,&myargv);

				//open target
				if((fp1 = fopen(myargv[0], "r+"))==NULL){
					printf("Failed to open the %s\n",mapperLine);
					perror("Failed mapperline open\n");
				}
				else{
					//get each line and increment the appropriate index corresponding to the letter
					while(fgets(textLine, 128, fp1)){
						charCount[toupper(textLine[0]) -'A'] += 1;

						//printf("textLine[%d] = %d\n", toupper(textLine[0]) -'A', charCount[toupper(textLine[0]) -'A']);
					}
				}
			}

			//FOR INFORMATION PURPOSES DELETE LATER/*
			/*for(i = 0; i < 26; i++){
				printf("%c = %d\n",i+'A', charCount[i]);
				strcat(letCount, i+'A' + '0');
			}*/
			//printf("# a's in %d = %d",i,charCount[0]);
			//char buff[] = "PIPES WORKING";
			//printf("Writing to pipe %d\n",2*i+1);
			write(mPipes[(2*i)+1], charCount, sizeof(charCount));
			close(mPipes[2*i+1]);

			return c_pid;
		}

	}


		printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^GOODBYE from phase2\n");

}
