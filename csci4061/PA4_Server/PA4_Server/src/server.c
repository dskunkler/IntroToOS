#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <zconf.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include "../include/protocol.h"

#define MAX_MSG_SIZE 1000

int azList[ALPHABETSIZE] = {0};
// 1st column: mapperID
// 2nd column: # updates
// 3rd column: check in/out
int updateStatus[MAX_STATUS_TABLE_LINES][3] = {0};

int currentConn = 0;
pthread_mutex_t currentConn_lock;
pthread_cond_t currentConn_cond;

struct threadArg {
    sem_t* azList_sem;
    sem_t* updateStatus_sem;
	int clientfd;
	char* clientip;
	int clientport;
};

void printList(void * arg) {
    struct threadArg * tArg = (struct threadArg *) arg;
    sem_wait(tArg->updateStatus_sem);
    printf("updateStatus: ");
    for (int i = 0; i < MAX_STATUS_TABLE_LINES; i++) {
        printf("%d%d%d ", updateStatus[i][0], updateStatus[i][1], updateStatus[i][2]);
    }
    printf("\n");
    sem_post(tArg->updateStatus_sem);
}

void clearBuf(int *buf) {
    for (int i = 0; i < RESPONSE_MSG_SIZE; i++) {
        buf[i] = 0;
    }
}

void clearLngBuf(int *buf){
    for(int i = 0; i < LONG_RESPONSE_MSG_SIZE; i++){
        buf[i] = 0;
    }
}

void * threadFunction(void * arg) {
	struct threadArg * tArg = (struct threadArg *) arg;
	int readbuf[REQUEST_MSG_SIZE] = {0};
	int responsebuf[RESPONSE_MSG_SIZE] = {0};
	int lngrespbuf[LONG_RESPONSE_MSG_SIZE] = {0};
	int id = -1;
	int num = 0;


    printf("open connection from %s:%d:%d\n", tArg->clientip, tArg->clientport, tArg->clientfd);
    while (1) {
        
		//attempt to read from client
        num = read(tArg->clientfd, readbuf, sizeof(int)*REQUEST_MSG_SIZE);
        

        if (num != -1) {
            //printf("sizeof(readbuf): %ld\n", sizeof(readbuf));

            //make sure mapper_id is valid
            if((readbuf[RQS_MAPPER_PID] > 0) || readbuf[RQS_MAPPER_PID] == -1 ){
              //if valid proceed with requests

              // 1: CHECKIN
              if (readbuf[RQS_COMMAND_ID] == CHECKIN && readbuf[RQS_MAPPER_PID]!= -1) {

                //if its not checked in, proceed
                if(updateStatus[id-1][US_IS_CHECKEDIN] != CHECKIN){

                  //set id for future requests and print properly
                  id = readbuf[RQS_MAPPER_PID];
                  printf("[%d] CHECKIN\n", id);

                  //update update status struct
                  sem_wait(tArg->updateStatus_sem);
                  updateStatus[id-1][US_MAPPER_PID] = id;
                  updateStatus[id-1][US_NUM_UPDATES] = 0;
                  updateStatus[id-1][US_IS_CHECKEDIN] = CHECKIN;
                  sem_post(tArg->updateStatus_sem);

                  //set response properly
                  responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
                  responsebuf[RSP_CODE] = RSP_OK;
                  responsebuf[RSP_DATA] = readbuf[RQS_MAPPER_PID];

                  //write to client and clear buff
                  write(tArg->clientfd, responsebuf, sizeof(responsebuf));
                  clearBuf(responsebuf);
                }

                //if its already checked in
                else{
                  printf("ALREADY CHECKED IN\n" );
                  //set response properly
                  responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
                  responsebuf[RSP_CODE] = RSP_NOK;
                  responsebuf[RSP_DATA] = readbuf[RQS_MAPPER_PID];

                  //write to client and clear buff
                  write(tArg->clientfd, responsebuf, sizeof(responsebuf));
                  clearBuf(responsebuf);

                }
              }

              // 2: UPDATE_AZLIST
              else if (readbuf[RQS_COMMAND_ID] == UPDATE_AZLIST && readbuf[RQS_MAPPER_PID]!= -1) {

                  //update azList from readbuf data
                  sem_wait(tArg->azList_sem);
                  for (int i = 0; i < ALPHABETSIZE; i++) {
                      azList[i] += readbuf[i+2];
                  }
                  sem_post(tArg->azList_sem);

                  //update UupdateStatus struct
                  sem_wait(tArg->updateStatus_sem);
                  updateStatus[id-1][US_NUM_UPDATES]++;
                  sem_post(tArg->updateStatus_sem);

                  //update response buffer
                  responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
                  responsebuf[RSP_CODE] = RSP_OK;
                  responsebuf[RSP_DATA] = readbuf[RQS_MAPPER_PID];

                  //write buffer to client then clear the buffer
                  write(tArg->clientfd, responsebuf, sizeof(responsebuf));
                  clearBuf(responsebuf);

                  //extra credit
                  if(readbuf[RQS_MAPPER_PID]== -1){
                      printf("CLOSING MASTER CLIENT CONNECTION\n");
                      close(tArg->clientfd);
                      printf("close connection from %s:%d:%d\n", tArg->clientip, tArg->clientport, tArg->clientfd);

                      //exit thread
                      pthread_exit(0);
                  }
              }

				// 3: GET_AZLIST
				else if (readbuf[RQS_COMMAND_ID] == GET_AZLIST) {
					printf("[%d] GET_AZLIST\n", id);

					//set lng resonse buffer
					lngrespbuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
					lngrespbuf[RSP_CODE] = RSP_OK;

					//lock and access azlist seeting response
					sem_wait(tArg->azList_sem);
					for (int i = 2; i < REQUEST_MSG_SIZE; i++) {
						lngrespbuf[i] = azList[i-2];
					}
					sem_post(tArg->azList_sem);

					//send response to client and clear buffer
					write(tArg->clientfd, lngrespbuf, sizeof(lngrespbuf));
					clearLngBuf(lngrespbuf);

					//extra credit: close for each command from master client
					if(readbuf[RQS_MAPPER_PID]== -1){

						close(tArg->clientfd);
						printf("close connection from %s:%d:%d\n", tArg->clientip, tArg->clientport, tArg->clientfd);

						//exit thread
						pthread_exit(0);
					}
              	}

				// 4: GET_MAPPER_UPDATES
				else if (readbuf[RQS_COMMAND_ID] == GET_MAPPER_UPDATES) {

					//Master Client cannot access this field
					if(readbuf[RQS_MAPPER_PID]!= -1){
						printf("[%d] GET_MAPPER_UPDATES\n", id);
						
						//set response buffer
						responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
						responsebuf[RSP_CODE] = RSP_OK;

						//lock and updatestatus
						sem_wait(tArg->updateStatus_sem);
						responsebuf[RSP_DATA] = updateStatus[id-1][US_NUM_UPDATES];
						sem_post(tArg->updateStatus_sem);

						//write response and clearbuff
						write(tArg->clientfd, responsebuf, sizeof(responsebuf));
						clearBuf(responsebuf);
					}
					else{
						//set response buffer
						responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
						responsebuf[RSP_CODE] = RSP_NOK;
						responsebuf[RSP_DATA] = 0; //Master client can never update
						
						//write response and clearbuff
						write(tArg->clientfd, responsebuf, sizeof(responsebuf));
						clearBuf(responsebuf);
					}
              	}

              // 5: GET_ALL_UPDATES
              else if (readbuf[RQS_COMMAND_ID] == GET_ALL_UPDATES) {
					printf("[%d] GET_ALL_UPDATES\n", id);
					responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
					responsebuf[RSP_CODE] = RSP_OK;
					responsebuf[RSP_DATA] = 0;
					sem_wait(tArg->updateStatus_sem);
					for (int i = 0; i < MAX_STATUS_TABLE_LINES; i++) {
						responsebuf[RSP_DATA] += updateStatus[i][US_NUM_UPDATES];
					}
					sem_post(tArg->updateStatus_sem);
					write(tArg->clientfd, responsebuf, sizeof(responsebuf));
					clearBuf(responsebuf);

                  //extra credit: close for each command from master client
					if(readbuf[RQS_MAPPER_PID]== -1){

						close(tArg->clientfd);
						printf("close connection from %s:%d:%d\n", tArg->clientip, tArg->clientport, tArg->clientfd);

						//exit thread
						pthread_exit(0);
					}
              }

              // 6: CHECKOUT
              else if (readbuf[RQS_COMMAND_ID] == CHECKOUT && readbuf[RQS_MAPPER_PID]!= -1) {
                //if correspending mapper id is not already checked out
                if(updateStatus[id-1][US_IS_CHECKEDIN]!= 0){

                  //Print desired output to terminal
                  printf("[%d] CHECKOUT\n", id);

                  //fill response buf appropriately
                  responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
                  responsebuf[RSP_CODE] = RSP_OK;
                  responsebuf[RSP_DATA] = readbuf[RQS_MAPPER_PID];

                  //Safely checkout in our table
                  sem_wait(tArg->updateStatus_sem);
                  updateStatus[id-1][US_IS_CHECKEDIN] = 0;
                  sem_post(tArg->updateStatus_sem);

                  //write it back to the client
                  write(tArg->clientfd, responsebuf, sizeof(responsebuf));
                  clearBuf(responsebuf);

                  //close connection
                  close(tArg->clientfd);
                  printf("close connection from %s:%d:%d\n", tArg->clientip, tArg->clientport, tArg->clientfd);

                  //exit thread
                  pthread_exit(0);
                }

                //if it is already checked out
                else{
                  //set response to RSP_NOK
                  responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
                  responsebuf[RSP_CODE] = RSP_NOK;
                  responsebuf[RSP_DATA] = readbuf[RQS_MAPPER_PID];

                  //write this back to client
                  write(tArg->clientfd, responsebuf, sizeof(responsebuf));
                  clearBuf(responsebuf);
                }
              }

              // Error, incorrect Command ID
              else if(readbuf[RQS_COMMAND_ID] < CHECKIN || readbuf[RQS_COMMAND_ID] > CHECKOUT){
					//printf("INCORRECT RQS COMMAND ID\n");
					//set error response
					lngrespbuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
					lngrespbuf[RSP_CODE] = RSP_NOK;
					lngrespbuf[RSP_DATA] = readbuf[RQS_MAPPER_PID];
					//response error
					write(tArg->clientfd, lngrespbuf, sizeof(lngrespbuf));
					clearLngBuf(lngrespbuf);

					//if master process tries to send improper command	
					if(readbuf[RQS_MAPPER_PID]== -1){

						close(tArg->clientfd);
						printf("close connection from %s:%d:%d\n", tArg->clientip, tArg->clientport, tArg->clientfd);

						//exit thread
						pthread_exit(0);
					}                 

              }

              //likely master client tried to access a request it didn't have permission to
              else {
					//set resp buff to incorrect
					//printf("ERROR SOMEWHERE WTF? CMN: %d PID: %d\n", readbuf[RQS_COMMAND_ID], readbuf[RQS_MAPPER_PID]);
					responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
					responsebuf[RSP_CODE] = RSP_NOK;
					responsebuf[RSP_DATA] = readbuf[RQS_MAPPER_PID];

					//inform client and clear buffer
					write(tArg->clientfd, responsebuf, sizeof(responsebuf));
					clearBuf(responsebuf);
					if(readbuf[RQS_MAPPER_PID] == -1){
						//close connection
						close(tArg->clientfd);
						printf("close connection from %s:%d:%d\n", tArg->clientip, tArg->clientport, tArg->clientfd);

						//exit thread
						pthread_exit(0);

                  	}
              	}
            }

          //Error, Incorrect mapper ID
			else{
				//notify incorrectness
				//printf("INCORRECT MAPPER ID = %d, cmd = %d\n",readbuf[RQS_MAPPER_PID],readbuf[RQS_COMMAND_ID]);

				//set response buf appropriately
				responsebuf[RSP_COMMAND_ID] = readbuf[RQS_COMMAND_ID];
				responsebuf[RSP_CODE] = RSP_NOK;
				responsebuf[RSP_DATA] = readbuf[RQS_MAPPER_PID];

				//notify client and clear buffer
				write(tArg->clientfd, responsebuf, sizeof(responsebuf));
				clearBuf(responsebuf);
				close(tArg->clientfd);

				//close connection. This mapper doesn't have the ability to make any changes within the thread and should exit
				printf("close connection from %s:%d:%d\n", tArg->clientip, tArg->clientport, tArg->clientfd);

				//exit thread
				pthread_exit(0);
			}
        }

		//clear buffers for next read
        memset(readbuf, 0, sizeof(readbuf));
        memset(responsebuf, 0, sizeof(responsebuf));

    }
  
	//safely decrement concurrent connection counter
  	pthread_mutex_lock(&currentConn_lock);
	currentConn--;
	pthread_cond_signal(&currentConn_cond);
	pthread_mutex_unlock(&currentConn_lock);
	return NULL;
}

int main(int argc, char *argv[]) {
    sem_t azList_sem;
    sem_t updateStatus_sem;
    sem_init(&azList_sem, 0, 1);
    sem_init(&updateStatus_sem, 0, 1);

    int server_port;

    if (argc == 2) { // 1 arguments
        server_port = atoi(argv[1]);
        printf("server_port: %d\n", server_port);
    } else {
        printf("Invalid or less number of arguments provided\n");
        printf("./server <server_Port>\n");
        exit(0);
    }

    // Server (Reducer) code
    pthread_t threads[MAX_CONCURRENT_CLIENTS];
	pthread_mutex_init(&currentConn_lock, NULL);
	pthread_cond_init(&currentConn_cond, NULL);

	// Create a TCP socket.
	int sock = socket(AF_INET , SOCK_STREAM , 0);

	// Bind it to a local address.
	struct sockaddr_in servAddress;
	servAddress.sin_family = AF_INET;
	servAddress.sin_port = htons(server_port);
	servAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int optval = 1;
    int r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (r != 0) {
        printf("Cannot enable SO_REUSEADDR option.\n");
    }
	bind(sock, (struct sockaddr *) &servAddress, sizeof(servAddress));

	// We must now listen on this port.
	listen(sock, MAX_CONCURRENT_CLIENTS);
    printf("server is listening\n");

	// A server typically runs infinitely, with some boolean flag to terminate.
	while (1) {

		// Now accept the incoming connections.
		struct sockaddr_in clientAddress;

		socklen_t size = sizeof(struct sockaddr_in);
		int clientfd = accept(sock, (struct sockaddr*) &clientAddress, &size);

        struct threadArg *arg = (struct threadArg *) malloc(sizeof(struct threadArg));

        arg->azList_sem = &azList_sem;
        arg->updateStatus_sem = &updateStatus_sem;
		arg->clientfd = clientfd;
		arg->clientip = inet_ntoa(clientAddress.sin_addr);
		arg->clientport = clientAddress.sin_port;
        //printf("open first connection from %s:%d:%d\n", arg->clientip, arg->clientport, arg->clientfd);

		//TODO: Handle the accepted connection by passing off functionality to a thread
		//      Up to MAX_CONCURRENT threads can be running simultaneously, so you will
		//      have to decide how to ensure that this condition holds.

		pthread_mutex_lock(&currentConn_lock);
		while (currentConn == MAX_CONCURRENT_CLIENTS) {
			//printf("server entered wait condition (currentConn = 0)\n");
			pthread_cond_wait(&currentConn_cond, &currentConn_lock);
		}
		currentConn++;
		pthread_create(&threads[currentConn], NULL, threadFunction, (void *) arg);
		pthread_mutex_unlock(&currentConn_lock);

	}

	// Close the socket.
	close(sock);

    //return 0;
}
