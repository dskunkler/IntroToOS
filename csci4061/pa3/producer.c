/*test machine: CSELAB_machine_name * date: mm/dd/yy
* name: full_name1 , [full_name2]
* x500: id_for_first_name , [id_for_second_name] */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
// pthread.h included in header.h

// Directive:
// 1)   Read the input file (line by line)
// 2)   Feed into shared queue 
//      Note: the shared queue is a linked list

// producer thread function
void * producer(void * nargs) 
{
    args_t * args = (args_t*) nargs;
    FILE *myfd;
    char *line;
    char textline[1024];
    char *filepath[1024];
    int error;
    int line_number = 0;

    if (args->p_option == true) {
        fprintf(args->logfd, "producer\n");
    }

    filepath[0] = args->filepath;

    if ((myfd = fopen(filepath[0], "r")) == NULL) {
        fprintf(stderr, "Failed to open file from structure.\n");
    }

    while (fgets(textline, 1025, myfd)) {
        if (args->p_option == true) {
            fprintf(args->logfd, "producer: %d\n", line_number);
        }
        
        line = (char*)malloc(sizeof(textline));
        line = strcpy(line, textline);

        //mutex encapsulate w/ queue_lock
        pthread_mutex_lock(args->queue_mutex);
        append_node(args->head, line, line_number); //CRITICAL CODE
        //args->node_count++; //CRITICAL CODE
        pthread_cond_signal(args->queue_cond);
        pthread_mutex_unlock(args->queue_mutex);

        line_number++;
    }
    
    pthread_mutex_lock(args->queue_mutex);
    args->num_working = 0; //CRITICAL CODE
    pthread_cond_signal(args->queue_cond);
    pthread_mutex_unlock(args->queue_mutex);

    fclose(myfd);
    printf("producer thread is exiting.\n");
    pthread_exit(0);
}
