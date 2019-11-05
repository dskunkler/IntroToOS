/*test machine: CSELAB_machine_name * date: mm/dd/yy
* name: full_name1 , [full_name2]
* x500: id_for_first_name , [id_for_second_name] */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
// pthread.h included in header.h

// Directive:
// 1)   Initialize the shared queue and result histogram global data structures (maybe put these in the header file?)
// 2)   Initialize the producer thread, 
// 3)   Initialize the consumer threads based on an argument given
// 

// int noConsumer, char filepath, char option
void main(int argc, char *argv[]) 
{
    // ./testing/test1.txt
    //argv[0]: name of the function call
    //argv[1]: no of Consumer threads to create
    //argv[2]: filepath to txt file with data to be parsed
    //argv[3]: option argument: "-p" for printing to log file
    //         The other to options are for extra credit

    //Check input arguments
    if(argc < 3){
        printf("Improper number of arguments.\n");
        exit(1);
    }

    pthread_t prod;
    args_t *p_args;
    p_args = (args_t*)malloc(sizeof(args_t));
    int i;
    printf("argv[1]: %s\n", argv[1]);
    pthread_t cons[atoi(argv[1])];
    args_t c_args[atoi(argv[1])];

    //initialize our hist array to 0's
    p_args->num_nodes = 0;
    for(i = 0; i < 26; i++){
        p_args->histogram[i] = 0;
    }

    //create a dummy node and set head and tail to it.
    struct node *shared_queue;
    create_dummy(&shared_queue);
    p_args->head = shared_queue;
    p_args->tail = shared_queue;
    

    int error;
    //pthread_mutex_t queue_lock, hist_lock;   
    p_args->queue_lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    p_args->hist_lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    p_args->cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));


    //producer thread needs the filepath and the shared queue structure
    p_args->filepath = argv[2];
    printf("p_args.filepath: %s\n", p_args->filepath);
    p_args->head = shared_queue;

    error = pthread_create(&prod, NULL, producer, (void *) p_args);
    pthread_join(prod, NULL);

    if (error == 0) {
        printf("No error creating producer thread\n");   
    }
    //append_node(p_args->head, shared_queue);
    //print_list(p_args->head);

    //consumer thread needs the shared queue structure and the result histogram structure
    for (int i = 0; i < atoi(argv[1]); i++) {
        c_args[i].head = shared_queue;
        error = pthread_create(&cons[i], NULL, consumer, (void *) p_args);
        pthread_join(cons[i], NULL);
        if (error == 0) {
            printf("cons[%d] is incomplete.\n", i);   
        }
    }
}