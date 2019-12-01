#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <zconf.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../include/protocol.h"

FILE *logfp;

void createLogFile(void) {
    pid_t p = fork();
    if (p==0)
        execl("/bin/rm", "rm", "-rf", "log", NULL); //removes log file

    wait(NULL);
    mkdir("log", ACCESSPERMS); //creates a new directory log
    logfp = fopen("log/log_client.txt", "w"); //creates new client log text file
}

int main(int argc, char *argv[]) {
    int mappers;
    int i, index;
    pid_t c_pid;
    int mapperID = 1;
    char numBuff[2];
    char target[256],cleantarg[256],line[256];
    char folderName[100] = {'\0'};
    char path[100] = "./MapperInput/Mapper_";
    char *server_ip;
    int server_port;
    FILE *fp, *fp1;
    int alphaCounter[26] = {0};

    if (argc == 5) { // 4 arguments
        strcpy(folderName, argv[1]);
        mappers = atoi(argv[2]);
        server_ip = argv[3];
        server_port = atoi(argv[4]);
        if (mappers > MAX_MAPPER_PER_MASTER) {
            printf("Maximum number of mappers is %d.\n", MAX_MAPPER_PER_MASTER);
            printf("./client <Folder Name> <# of mappers> <server IP> <server Port>\n");
            exit(1);
        }

    } else {
        printf("Invalid or less number of arguments provided\n");
        printf("./client <Folder Name> <# of mappers> <server IP> <server Port>\n");
        exit(1);
    }

    // create log file
    createLogFile();

    // phase1 - File Path Partitioning
    traverseFS(mappers, folderName);

    // Phase2 - Mapper Clients's Deterministic Request Handling
    //create mappers
    for(i = 0; i < mappers; i++){
        c_pid = fork();

        //some error occured
        if(c_pid < 0){
            perror("Error forking\n");
        }
        //increment mapperID so our new child has a new and correct ID
        if(c_pid > 0){
            mapperID++;
            wait(NULL); //DO WE WANT TO WAIT? WILL THAT EFFECT PARRALELISM? SHOULD WE MOVE THIS TO LATER?
        }

        //this is a client process
        if(c_pid == 0){
            //create correct name
            sprintf(numBuff, "%d",mapperID);
            strcat(path, numBuff);
            strcat(path, ".txt");
            int rspbuf[LONG_RESPONSE_MSG_SIZE] = {0};




            //lets open mapperfile
            if((fp = fopen(path, "r")) == NULL){
              perror("Error opening filepath\n");
              return 1;
            }
            //create a TCP socket
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            //specify connect to address
            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_port = htons(server_port);
            address.sin_addr.s_addr = inet_addr(server_ip);

            //Connect to it
            if(connect(sockfd, (struct sockaddr*) &address, sizeof(address))==0){
              printf("Connected\n");
              printf("***********************ID********************* = %d\n", mapperID);
              //after connected create the msg buffer
              int msgbuf[REQUEST_MSG_SIZE] = {0};
              msgbuf[0] = 1;
              msgbuf[1] = mapperID;
              write(sockfd,msgbuf,sizeof(msgbuf));


              //do some shit in here based on the specs



            //traverses the path file creating targets to open
            while(fgets(target, sizeof(target), fp)){

              //cleaning target up so there aren't empty ssspaces at teh end
              sscanf(target, "%s", cleantarg);
            //  printf("clean targ = %s\n", cleantarg);
              //open our cleaned up target file
              if((fp1 = fopen(cleantarg, "r")) == NULL){
                perror("Error opening target\n");
              }

              //traverse our target file line by line. get first letter of first word and increment our alphaCounter
              else{

                //printf("opened %s sucessfully\n", cleantarg);
                while(fgets(line, sizeof(line), fp1)){
                  //printf("Adding letter %c\n",line[0]);
                  index = toupper(line[0])-'A';
                  //printf("index = %d\n", index);
                  //printf("before::: alfCounter[%d] = %d\n",index,alphaCounter[index]);
                  alphaCounter[index]++;
                  //printf("after: alfCounter[%d] = %d", index,alphaCounter[index]);
                }

                //set msgbuff correctly
                msgbuf[0] = 2;
                msgbuf[1] = mapperID;
                for(i = 0; i< ALPHABETSIZE; i++){
                  msgbuf[i+2] = alphaCounter[i];
                }
                //write data to our server
                write(sockfd, msgbuf,sizeof(msgbuf));

                //to be deleted
                for(i = 0; i < 26; i++){
                  printf("%c = %d\n", i+ 'A', alphaCounter[i]);
                }

                //reset our counter for the next pass
                memset(alphaCounter, 0, sizeof(alphaCounter));

              }

            }


            //Request 3

            //clear msgbuff
            memset(msgbuf,0, sizeof(msgbuf));
            //set buff to correct val's for request 3
            msgbuf[0] = 3;
            msgbuf[1] = mapperID;
            //write request
            write(sockfd, msgbuf, sizeof(msgbuf));
            //read response
            read(sockfd,rspbuf, 28 * sizeof(int));

            //Request 4

            
            close(sockfd);
          }

          else{
            perror("Connection failed because\n");
          }



              return 0;
        }

    }


    // Phase3 - Master Client's Dynamic Request Handling (Extra Credit)


    fclose(logfp);
    return 0;

}
