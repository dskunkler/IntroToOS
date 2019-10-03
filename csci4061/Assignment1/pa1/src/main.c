#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "graph.h"
#include "dfs_stack.h"
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


void dfsPrint(target* t, int start, int size)
{
  //printf("starting dfs\n");
  int i = 0, j = 0, k = 0;
  char* d;
  if(!t[start].visited){
    for(i = 0; i< t[start].dep_count; i++){
      d = t[start].depend[i];
      for(j = 0; j < size; j++){
        if(!strcmp(t[j].name, d)){
					//printf("found the names index\n");
          dfsPrint(t,j,size);
        }
      }
    }
    for(k = 0; k < t[start].recipe_count; k++){
      //run the recipe.
      //printf("printing the recipe\n");
      printf("%s\n", t[start].recipe[k]);
    }
    t[start].visited = 1;
    //printf("marked as visited\n");
  }
  //printf("bye bye from dfs\n");
}




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


void dfs(target* t, int start, int size)
{
  //printf("starting dfs\n start = %d\n size = %d\n", start, size);
  int i = 0, j = 0, k = 0;
  char* d;
  if(!t[start].visited){
		//printf("not visited\n");
    for(i = 0; i< t[start].dep_count; i++){
		//printf("depends = \n");
      d = t[start].depend[i];
      for(j = 0; j < size; j++){
				//printf("comparing j = %d\n t[j].name = %s\n", j, t[j].name);
        if(!strcmp(t[j].name, d)){
          //printf("found the names index\n");
          dfs(t,j,size);
        }
      }
    }
		char delim[] = " :\t\n";
		char **myargv;
    for(k = 0; k < t[start].recipe_count; k++){
      //run the recipe.
      //printf("printing the recipe\n");
      //printf("recipe = %s\n", t[start].recipe[k]);
			makeargv(t[start].recipe[k], delim, &myargv);
			/*
			printf("args = \n");
			int z = 0;
			for(z = 0; z< ma; z++){
				printf("%s\n", myargv[z]);
			}*/
			pid_t child = fork();
			if(child < 0){
				printf("Error in fork.\n");
				exit(1);
			}
			else if(child > 0){
				wait(NULL);

			}
			else{
				execvp(myargv[0], myargv);
				perror("THAT DIDNT WORK");	
			}		
    }
    t[start].visited = 1;
    //printf("marked as visited\n");
  }
  //printf("bye bye from dfs\n");
}


//Parse the input makefile to determine targets, dependencies, and recipes
int process_file(char *fname)
{
	FILE* fp = fopen(fname, "r");
	char line[LINE_SIZE];
	int i = 0;

	if (!fp) {
		printf("Failed to open the file: %s \n", fname);
		return -1;
	}

	//Read the contents and store in lines
	while (fgets(line, LINE_SIZE, fp))
		strncpy(lines[i++], line, strlen(line));

	fclose(fp);

	return 0;
}


//Validate the input arguments, bullet proof the program
int main(int argc, char *argv[])
{
	//$./mymake Makefile
	//Similarly account for -r flag
	if (argc == 1){
		printf("Not enough arguments.\n");
		exit(1);
	}

	if (argc > 3){
		printf("Too many arguments\n");
		exit(1);
	}
	if((argc == 2 && !strncmp(argv[1], "-p", 2 ))||(argc == 2 && !strncmp(argv[1], "-r", 2 ))){
		printf("Not enough arguments. Need makefile after flags.\n");
		exit(1);
	}

	if (argc == 2 && strncmp(argv[1], "-p", 2) && strncmp(argv[1], "-r", 2)) //only 2 args and no flags
  {
    //printf("no flags\n" );

		process_file(argv[1]);
		int i, numTargs = 0; //
		for(i = 0; i < sizeof(lines)/sizeof(lines[1]); i++){
		  //printf("%d: ",i);
			//printf("%s\n", lines[i]);
			if(strstr(lines[i], ":") != NULL){
				numTargs++;
			}
		}
		//printf("Numbers of targets = %d\n", numTargs);
		target t[numTargs]; //t is an array of graphs.
		i= 0;
		int k;
		int j = -1;
		char delim[] = " :\t\n";     //delimiting chars
		char **myargv;              //array of tokens from line
		int ma;
		for(i = 0; i < MAX_LINES; i++){
			if(strstr(lines[i], ":")!= NULL){ //if lines has : in it
				//printf("%d\n",j);
				ma = makeargv(lines[i],delim,&myargv); // ma is num tokens
				//printf(" myargv[0] = %s\n", myargv[0]);
				//printf("j = %d\n", j);

				t[++j].name = myargv[0]; // first one is target
        t[j].recipe_count = 0;
        t[j].dep_count = 0;
        t[j].visited = 0;
				//printf(" t[j].name = %s\n", t[j].name);
				for(k = 0; k < ma; k++){ //rest are dependencies
					t[j].depend[k] = myargv[k+1];
				}
				t[j].dep_count = ma-1;
			}
			else{
				char recStr[LINE_SIZE] = "";
				int ra;
        if(strstr(lines[i], "\t")!= NULL){
					ma = makeargv(lines[i], delim, &myargv); //tokenize the line
					//printf("%d\n", ma);
					strcat(recStr, myargv[0] ); //create the cleaned up recipe string
					//printf("%s\n", recStr);
					for(ra = 1; ra < ma; ra++){
						strcat(recStr, " ");
						strcat(recStr, myargv[ra]);					
					}
					
					t[j].recipe[t[j].recipe_count] = malloc(sizeof(recStr)); //allocate space for the recipe
					strcat(t[j].recipe[t[j].recipe_count], recStr); //put the recipe in there
  				t[j].recipe_count++; //increase number of recipes counted
  			}
      }

		}
		dfs(t,0,numTargs);
		/*
		printf("%s\n", t[0].name);
		for(i = 0; i<numTargs; i++){
			printf("targ = %s",t[i].name);
			printf("\ndependencies = ");
			for(j = 0;j < t[i].dep_count; j++){
				printf("%s ", t[i].depend[j]);
			}
      printf("\nRecipes:  %d\n", t[i].recipe_count);

      for(k = 0; k < t[i].recipe_count; k++){
        printf("recipe = %s\n",t[i].recipe[k]);
      }
      printf("\n");
		}*/
  }
//YEHAW IT WORKS
		






	if (argc == 3) {
		//$./mymake Makefile target
    if(!strncmp(argv[1], "-r", 2)){
      if(!process_file(argv[2])){

				//printf("-r flag\n");

				int i, numTargs = 0; 

				for(i = 0; i < sizeof(lines)/sizeof(lines[1]); i++)
				{
					//printf("%d: ",i);
					//printf("%s\n", lines[i]);

					if(strstr(lines[i], ":") != NULL)
					{
						numTargs++;
					}
				}
				//printf("Numbers of targets = %d\n", numTargs);
				target t[numTargs]; //t is an array of graphs.
				i= 0;
				int k;
				int j = -1;
				char delim[] = " :\t\n";     //delimiting chars
				char **myargv;              //array of tokens from line
				int ma;
				for(i = 0; i < MAX_LINES; i++){
					if(strstr(lines[i], ":")!= NULL){ //if lines has : in it
					//	printf("%d\n",j);
						ma = makeargv(lines[i],delim,&myargv); // ma is num tokens
						//printf(" myargv[0] = %s\n", myargv[0]);
						//printf("j = %d\n", j);
						if(j >= 1){
						//	printf("name before = %s\n", t[j-1].name);
						}
						t[++j].name = myargv[0]; // first one is target
						t[j].recipe_count = 0;
						t[j].dep_count = 0;
						t[j].visited = 0;
						//printf(" t[j].name = %s\n", t[j].name);
						for(k = 0; k < ma; k++){ //rest are dependencies
							t[j].depend[k] = myargv[k+1];
						}
						t[j].dep_count = ma-1;
					}
					else{
						char recStr[LINE_SIZE] = "";
						int ra;
						if(strstr(lines[i], "\t")!= NULL){

							ma = makeargv(lines[i], delim, &myargv); //tokenize the line
							//printf("%d\n", ma);
							strcat(recStr, myargv[0] ); //create the cleaned up recipe string
							//printf("%s\n", recStr);
							for(ra = 1; ra < ma; ra++){
								strcat(recStr, " ");
								strcat(recStr, myargv[ra]);					
							}
							
							t[j].recipe[t[j].recipe_count] = malloc(sizeof(recStr)); //allocate space for the recipe
							strcat(t[j].recipe[t[j].recipe_count], recStr); //put the recipe in there
							t[j].recipe_count++; //increase number of recipes counted
						}
					}
				}
        //printf("%s\n", t[0].name);
        dfsPrint(t,0,numTargs);
      }
    }

		else if (strncmp(argv[1], "-p", 2) && !process_file(argv[1])) {

      
      char* makeTarget = argv[2];
		
      int i= 0, k= 0, j = -1, numTargs = 0, ma;
  		char delim[] = " :\t\n";     //delimiting chars
  		char **myargv;              //array of tokens from line

  		for(i = 0; i < sizeof(lines)/sizeof(lines[1]); i++){ //Finds the number of targets in the file
  		  
  			if(strstr(lines[i], ":") != NULL){
  				numTargs++;
  			}				
  		}
  		
  		target t[numTargs]; //t is an array of graphs.


      for(i = 0; i < MAX_LINES; i++){

  			if(strstr(lines[i], ":")!= NULL){ //if lines has : in it
  				ma = makeargv(lines[i],delim,&myargv); // ma is num tokens
  				t[++j].name = myargv[0]; // first one is target
          t[j].recipe_count = 0;
          t[j].dep_count = 0;
          t[j].visited = 0;

  				for(k = 0; k < ma; k++){ //rest are dependencies
  					t[j].depend[k] = myargv[k+1];
  				}

  				t[j].dep_count = ma-1;
  			}
  			else{

          char recStr[LINE_SIZE] = "";
					int ra;

					if(strstr(lines[i], "\t")!= NULL){
						ma = makeargv(lines[i], delim, &myargv); //tokenize the line
						strcat(recStr, myargv[0] ); //create the cleaned up recipe string
						for(ra = 1; ra < ma; ra++){
							strcat(recStr, " ");
							strcat(recStr, myargv[ra]);					
						}
						
						t[j].recipe[t[j].recipe_count] = malloc(sizeof(recStr)); //allocate space for the recipe
						strcat(t[j].recipe[t[j].recipe_count], recStr); //put the recipe in there
						t[j].recipe_count++; //increase number of recipes counted
					}
        }
  	  }
			int hasTarg = 0;
			for(j = 0; j < numTargs; j++){
				//printf("comparing j = %d\n t[j].name = %s\n", j, t[j].name);
        if(!strcmp(t[j].name, makeTarget)){
					hasTarg = 1;
          dfs(t,j,numTargs);
        }
      }
			if(hasTarg == 0){
				printf("No such target in Makefile.\n");
				exit(1);
			}


		}
    else if (!strncmp(argv[1], "-p", 2)) {
			//$./mymake -p Makefile
			if (!process_file(argv[2]))
      {

        //printf("-p flag\n");

    		int i, numTargs = 0; //
    		for(i = 0; i < sizeof(lines)/sizeof(lines[1]); i++){
    		  //printf("%d: ",i);
    			//printf("%s\n", lines[i]);
    			if(strstr(lines[i], ":") != NULL){
    				numTargs++;
    			}
    		}
    		//printf("Numbers of targets = %d\n", numTargs);
    		target t[numTargs]; //t is an array of graphs.
    		i= 0;
    		int k;
    		int j = -1;
    		char delim[] = " :\t\n";     //delimiting chars
    		char **myargv;              //array of tokens from line
    		int ma;
    		for(i = 0; i < MAX_LINES; i++){
    			if(strstr(lines[i], ":")!= NULL){ //if lines has : in it
    				//printf("%d\n",j);
    				ma = makeargv(lines[i],delim,&myargv); // ma is num tokens
    				//printf(" myargv[0] = %s\n", myargv[0]);
    				//printf("j = %d\n", j);
    				t[++j].name = myargv[0]; // first one is target
            t[j].recipe_count = 0;
            t[j].dep_count = 0;
            t[j].visited = 0;
    				//printf(" t[j].name = %s\n", t[j].name);

    				for(k = 0; k < ma; k++){ //rest are dependencies
    					t[j].depend[k] = myargv[k+1];
    				}

    				t[j].dep_count = ma-1;
    			}
    			else{
						char recStr[LINE_SIZE] = "";
						int ra;
						if(strstr(lines[i], "\t")!= NULL){
							ma = makeargv(lines[i], delim, &myargv); //tokenize the line
						//	printf("%d\n", ma);
							strcat(recStr, myargv[0] ); //create the cleaned up recipe string
							//printf("%s\n", recStr);
							for(ra = 1; ra < ma; ra++){
								strcat(recStr, " ");
								strcat(recStr, myargv[ra]);					
							}
							
							t[j].recipe[t[j].recipe_count] = malloc(sizeof(recStr)); //allocate space for the recipe
							strcat(t[j].recipe[t[j].recipe_count], recStr); //put the recipe in there
							t[j].recipe_count++; //increase number of recipes counted
						}
          }
    		}
        for(i = 0; i< numTargs; i++){
          printf("target '%s' has %d dependencies and %d recipe\n",t[i].name, t[i].dep_count, t[i].recipe_count );
          for(j = 0; j< t[i].dep_count; j ++){
            printf("Dependency %d is %s\n" ,j, t[i].depend[j] );
          }
          for(k = 0; k < t[i].recipe_count; k++){
            printf("Recipe %d is %s\n", k, t[i].recipe[k]);
          }
          printf("\n");
        }
			}
    }
  }



	exit(EXIT_SUCCESS);
}
