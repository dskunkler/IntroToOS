#define _BSD_SOURCE
#define _DEFAULT_SOURCE
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
#include <sys/stat.h>
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

void errorResponse(int *resp) {
    if (resp[RSP_DATA] == RSP_NOK)
        printf("ERROR RESPONSE FROM SERVER");
}

int main(int argc, char *argv[]) {
    int mappers;
    int i, index;
    pid_t c_pid;
    int mapperID = 0;
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
    printf("mappers: %d\n", mappers);
    // create log file
    createLogFile();
    //change stdout to log.txt
    int d2, fd;
    fd = fileno(logfp);
    d2 = dup2(fd, STDOUT_FILENO);
    if(d2 == -1){
      perror("ERROR IN DUP2\n");
      return 1;
    }

    // phase1 - File Path Partitioning
    traverseFS(mappers, folderName);

    // Phase2 - Mapper Clients's Deterministic Request Handling
    //create mappers
    for(i = 0; i < mappers; i++){
        mapperID++;
        c_pid = fork();
        if(c_pid == 0)
            break;
        //some error occured
        if(c_pid < 0){
            perror("Error forking\n");
        }
    }

    if(c_pid > 0){
        mapperID = -1;
        wait(NULL); // Master Client process waits
        //printf("mapperID: %d\n", mapperID);
    }


    //this is a Mapper Client process
    if(c_pid == 0){


        //create correct filename
        sprintf(numBuff, "%d", mapperID);
        strcat(path, numBuff);
        strcat(path, ".txt");

        //lets open mapperfile
        if((fp = fopen(path, "r")) == NULL){
            perror("Error opening filepath\n");
            return 1;
        }

        //create a TCP socket
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);

        //specify address struct
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(server_port);
        address.sin_addr.s_addr = inet_addr(server_ip); //note inet_addr returns -1 on failure a valid ip addr

        //Connect to it
        if(connect(sockfd, (struct sockaddr*) &address, sizeof(address))==0){
            printf("[%d] open connection\n", mapperID);

            //after connected create the msg buffer
            int msgbuf[REQUEST_MSG_SIZE] = {0};
            int responsebuf[RESPONSE_MSG_SIZE] = {0};
            int lngrespbuf[LONG_RESPONSE_MSG_SIZE] = {0};

            // 1: CHECKIN
            //printf("[%d] CHECKIN\n", mapperID);
            memset(msgbuf, 0, sizeof(msgbuf));
            memset(responsebuf, 0, sizeof(responsebuf));

            //specify accurate request
            msgbuf[RQS_COMMAND_ID] = CHECKIN;
            msgbuf[RQS_MAPPER_PID] = mapperID;

            //write request to server then read response
            write(sockfd, msgbuf, sizeof(msgbuf));
            read(sockfd, responsebuf, sizeof(int)*REQUEST_MSG_SIZE);

            //error check response
            if(responsebuf[RSP_CODE] == RSP_NOK){
              printf("ERROR AT CHECKIN");
              return 1;
            }

            //print properly to log
            printf("[%d] CHECKIN: %d %d\n", mapperID, responsebuf[RSP_CODE], responsebuf[RSP_DATA]);


            // 2: UPDATE_AZLIST
            memset(msgbuf, 0, sizeof(msgbuf));
            memset(responsebuf, 0, sizeof(responsebuf));
            int updateTotal = 0;
            while(fgets(target, sizeof(target), fp) != NULL){
                //printf("entered: %s", target);

                //cleaning target up so there aren't empty ssspaces at teh end
                sscanf(target, "%s", cleantarg);
                //printf("clean targ = %s\n", cleantarg);
                //open our cleaned up target file
                if((fp1 = fopen(cleantarg, "r")) == NULL){
                    perror("Error opening target\n");
                }

                    //printf("opened %s sucessfully\n", cleantarg);
                    //traverse our target file line by line, get first letter of word
                    // and increment our alphaCounter
                    while(fgets(line, sizeof(line), fp1)){
                        //printf("Adding letter %c\n",line[0]);
                        index = toupper(line[0])-'A';
                        //printf("index = %d\n", index);
                        //printf("before::: alfCounter[%d] = %d\n",index,alphaCounter[index]);
                        alphaCounter[index]++;
                        //printf("after: alfCounter[%d] = %d", index,alphaCounter[index]);
                    }

                    //set msgbuf correctly
                    msgbuf[RQS_COMMAND_ID] = UPDATE_AZLIST;
                    msgbuf[RQS_MAPPER_PID] = mapperID;
                    for(i = 0; i < ALPHABETSIZE; i++){
                        msgbuf[i + 2] = alphaCounter[i];
                        //printf("%c = %d\n", i+ 'A', msgbuf[i+2]);
                    }

                    //write data to our server, check response
                    write(sockfd, msgbuf, sizeof(msgbuf));
                    read(sockfd, responsebuf, sizeof(int)*RESPONSE_MSG_SIZE);

                    //error check response
                    if(responsebuf[RSP_CODE] == RSP_NOK){
                      printf("ERROR AT UPDATE_AZLIST\n");
                      return 1;
                    }

                    //reset our counter for the next pass
                    memset(alphaCounter, 0, sizeof(alphaCounter));
                    memset(msgbuf, 0, sizeof(msgbuf));
                    memset(responsebuf, 0, sizeof(responsebuf));

                updateTotal++;
            }

            //print desired structure to log
            printf("[%d] UPDATE_AZLIST: %d\n", mapperID, updateTotal);

            // 3: GET_AZLIST

            //reset buffers
            memset(msgbuf, 0, sizeof(msgbuf));
            memset(lngrespbuf, 0, sizeof(lngrespbuf));

            //set request
            msgbuf[RQS_COMMAND_ID] = GET_AZLIST;
            msgbuf[RQS_MAPPER_PID] = mapperID;

            //Send request to server, monitor response
            write(sockfd, msgbuf, sizeof(msgbuf));
            read(sockfd, lngrespbuf, sizeof(int)*LONG_RESPONSE_MSG_SIZE);

            //error check response
            if(lngrespbuf[RSP_CODE] == RSP_NOK){
              printf("ERROR AT GET_AZLIST\n");
              return 1;
            }

            //do appropriate printing to log
            printf("[%d] GET_AZLIST: %d  ", mapperID, lngrespbuf[RSP_CODE]);
            for (int i = 2; i < LONG_RESPONSE_MSG_SIZE; i++) {
                printf("%d ", lngrespbuf[i]);
            }
            printf("\n");

            // 4: GET_MAPPER_UPDATES
            //printf("[%d] GET_MAPPER_UPDATES\n", mapperID);

            //reset buff's
            memset(msgbuf, 0, sizeof(msgbuf));
            memset(responsebuf, 0, sizeof(responsebuf));

            //set buffers
            msgbuf[RQS_COMMAND_ID] = GET_MAPPER_UPDATES;
            msgbuf[RQS_MAPPER_PID] = mapperID;

            //send request to server, read response
            write(sockfd, msgbuf, sizeof(msgbuf));
            read(sockfd, responsebuf, sizeof(int)*REQUEST_MSG_SIZE);

            //error check response
            if(responsebuf[RSP_CODE] == RSP_NOK){
              printf("ERROR AT GET_MAPPER_UPDATES\n");
              return 1;
            }

            //print appropriately to log
            printf("[%d] GET_MAPPER_UPDATES: %d %d\n", mapperID, responsebuf[RSP_CODE], responsebuf[RSP_DATA]);


            // 5: GET_ALL_UPDATES
            //printf("[%d] GET_ALL_UPDATES\n", mapperID);

            //clear buffers
            memset(msgbuf, 0, sizeof(msgbuf));
            memset(responsebuf, 0, sizeof(responsebuf));

            //set buffers
            msgbuf[RQS_COMMAND_ID] = GET_ALL_UPDATES;
            msgbuf[RQS_MAPPER_PID] = mapperID;

            //write to server and read response
            write(sockfd, msgbuf, sizeof(msgbuf));
            read(sockfd, responsebuf, sizeof(int)*REQUEST_MSG_SIZE);

            //error check response
            if(responsebuf[RSP_CODE] == RSP_NOK){
              printf("ERROR AT GET_ALL_UPDATES\n");
              return 1;
            }

            //print appropriately to log
            printf("[%d] GET_ALL_UPDATES: %d %d\n", mapperID, responsebuf[RSP_CODE], responsebuf[RSP_DATA]);


            // 6: CHECKOUT
            //printf("[%d] CHECKOUT\n", mapperID);

            //reset buffer
            memset(msgbuf, 0, sizeof(msgbuf));
            memset(responsebuf, 0, sizeof(responsebuf));

            //set buffer
            msgbuf[RQS_COMMAND_ID] = CHECKOUT;
            msgbuf[RQS_MAPPER_PID] = mapperID;

            //write to server, read response
            write(sockfd, msgbuf, sizeof(msgbuf));
            read(sockfd, responsebuf, sizeof(int)*REQUEST_MSG_SIZE);

            //error check response
            if(responsebuf[RSP_CODE] == RSP_NOK){
              printf("ERROR AT CHECKOUT\n");
              return 1;
            }

            //print appropriately to log
            printf("[%d] CHECKOUT: %d %d\n", mapperID, responsebuf[RSP_CODE], responsebuf[RSP_DATA]);

            //close connection
            printf("[%d] close connection\n", mapperID);
            close(sockfd);
        }

        else{
            perror("Connection failed because\n");
        }
        return 0;
    }

    // Phase3 - Master Client's Dynamic Request Handling (Extra Credit)

    wait(NULL);

    FILE *cmdfp;
    memset(target, 0, sizeof(target));
    int mastercmd = 0;

    if((cmdfp = fopen("./commands.txt", "r")) ==NULL){ //creates new client log text file
      perror("ERROR OPENING COMMANDS.TXT\n");
    }

    // create the msg buffer
    int msgbuf[REQUEST_MSG_SIZE] = {0};
    int responsebuf[RESPONSE_MSG_SIZE] = {0};
    int lngrespbuf[LONG_RESPONSE_MSG_SIZE] = {0};

    //create a TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //specify address struct
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(server_port);
    address.sin_addr.s_addr = inet_addr(server_ip); //note inet_addr returns -1 on failure a valid ip addr

    while(fgets(target, sizeof(target), cmdfp) != NULL){
      //clean up line and change to an int
      sscanf(target, "%s", cleantarg);
      //printf("%s\n",cleantarg);
      mastercmd = atoi(cleantarg);

      //reset the buffers
      memset(msgbuf,0,sizeof(msgbuf));
      memset(responsebuf, 0, sizeof(responsebuf));
      memset(lngrespbuf, 0 , sizeof(lngrespbuf));

      //I'm not sure why I need to do this? Maybe the port is busy after the last close or close just totally annihilates the socket.
      sockfd = socket(AF_INET, SOCK_STREAM, 0);

      //if we can connect to the server
      	if(connect(sockfd, (struct sockaddr*) &address, sizeof(address))==0){
			//print appropriately and set buffer
			printf("[%d] open connection\n", mapperID);
			msgbuf[RQS_COMMAND_ID] = mastercmd;
			msgbuf[RQS_MAPPER_PID] = mapperID;

			//write to server
			//printf("hello from parent. cmd = %d, id = %d\n",msgbuf[RQS_COMMAND_ID], msgbuf[RQS_MAPPER_PID]);
			write(sockfd, msgbuf, sizeof(msgbuf));

			//GET_AZLIST requires a lngresponse buffer, read from server
			if(mastercmd == GET_AZLIST){
				read(sockfd,lngrespbuf, sizeof(int) * LONG_RESPONSE_MSG_SIZE);
				
			}
			else{
				read(sockfd, responsebuf, sizeof(int)*REQUEST_MSG_SIZE);
			}

			switch(mastercmd){
				case CHECKIN:
					printf("[%d] CHECKIN: %d %d\n", mapperID, responsebuf[RSP_CODE], responsebuf[RSP_DATA]);
					break;

				case GET_AZLIST:
					printf("[%d] GET_AZLIST: %d ", mapperID, lngrespbuf[RSP_CODE]);
					for (int i = 2; i < LONG_RESPONSE_MSG_SIZE; i++) {
					printf("%d ", lngrespbuf[i]);
				}
				printf("\n");
					break;

				case GET_MAPPER_UPDATES:
					printf("[%d] GET_MAPPER_UPDATES: %d %d\n", mapperID, responsebuf[RSP_CODE], responsebuf[RSP_DATA]);
					break;

				case GET_ALL_UPDATES:
					printf("[%d] GET_ALL_UPDATES: %d %d\n", mapperID, responsebuf[RSP_CODE], responsebuf[RSP_DATA]);
					break;

				case CHECKOUT:
					printf("[%d] CHECKOUT: %d %d\n", mapperID, responsebuf[RSP_CODE], responsebuf[RSP_DATA]);
					break;

				default:
				printf("[%d] wrong command\n",mapperID);
			}

			printf("[%d] close connection\n", mapperID);
			close(sockfd);
		}

        else{
          printf("CONNECTION FAILED\n");
          close(sockfd);
        }
    }

	//Close all file drives
	
    fclose(cmdfp);
    fclose(logfp);
    return 0;

}
