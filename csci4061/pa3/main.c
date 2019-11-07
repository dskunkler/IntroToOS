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

// main thread
void main(int argc, char *argv[]) 
{
    // ./testing/test1.txt
    //argv[0]: name of the function call
    //argv[1]: no of Consumer threads to create
    //argv[2]: filepath to txt file with data to be parsed
    //argv[3]: option argument: "-p" for printing to log file
    //argv[4]: no of the queue_size for bounded buffer
    //         The other to options are for extra credit
    
    char *ptr;
    long num_threads;
    num_threads = strtol(argv[1], &ptr, 10);
    printf("num_threads is %ld\n", num_threads);
    pthread_t prod;
    pthread_t cons[num_threads];
    args_t args;
    args_t c_args[num_threads]; // trying to give consumers their own id #
    FILE *testfd;
    FILE *logfd;
    FILE *rfd;
    int error;
    int i;

    //printf("argc = %d\n", argc);
    //The following 4 if statements perfom validation on the input arguments
    if (argc < 3 || argc > 5) {
        if (argc < 3) {
            printf("ERROR: Too few arguments.\n");
            return;
        }
        if (argc > 5) {
            printf("ERROR: Too many arguments.\n");
            return;
        }
    }

    if (argc >= 3) {
        //check that argv[1] is a number greater than zero
        if (num_threads <= 0) {
            printf("ERROR: first argument must only be an integer greater than zero.\n");
            return;
        }
        //check argv[2] is a filepath that opens
        char *str;
        int len;
        len = strlen(argv[2]);
        str = (char*)malloc(len);
        strcpy(str, argv[2]);
        if ((testfd = fopen(str, "r")) == NULL) {
            printf("ERROR: second argument must be an accurate filepath.\n");
            return;
        }
    }

    if (argc >= 4) {
        //check that argv[3] is an approved option (-p, -b, -bp)
        int i = !strcmp(argv[3], "-p");
        int j = !strcmp(argv[3], "-b");
        int k = !strcmp(argv[3], "-bp");
        //printf("result of strncmp is %d_%d_%d\n", i, j, k);

        if (i != 1 && j != 1 && k != 1) {
            printf("ERROR: third argument must be a correct option flag \"-p\", \"-b\", or \"-bp\". \n");
            return;
        }
    }

    if (argc == 5) {
        //check that argv[4] is a number greater than zero
        //and check that the option flag is -b or -bp
        if (!strcmp(argv[3], "-b") || !strcmp(argv[3], "-bp")) {
            long b_size;
            b_size = strtol(argv[4], &ptr, 10);
            if (b_size <= 0) {
                printf("ERROR: fourth argument must be an integer greater than zero.\n");
                return;
            }
        } else {
            printf("ERROR: only options \"-b\" and \"-bp\" support bounded buffers.\n");
            return;
        }
    }

    //sets up the log.txt file descriptor for the "-p" flag 
    if (argc == 4) {
        if (!strcmp(argv[3], "-p")) {
            args.p_option = true;
            if ((logfd = fopen("log.txt", "w")) == NULL) {
                fprintf(stderr, "Failed to open file from structure.\n");
            }
            args.logfd = logfd;
        } else {
            args.p_option = false;
            args.b_option = false;
            args.bp_option = false;
        }
    } 
    //printf("args.buffer_size = %ld\n", args.buffer_size);

    //sets up the queue buffer for the "-b" or "-bp" flag 
    // and the log.txt file descriptor for the "-bp" flag
    if (argc == 5) {
        if (!strcmp(argv[3], "-b")) {
            args.b_option = true;
            long b_size;
            b_size = strtol(argv[4], &ptr, 10);
            args.buffer_size = b_size;
        } else if (!strcmp(argv[3], "-bp")) {
            args.p_option = true;
            if ((logfd = fopen("log.txt", "w")) == NULL) {
                fprintf(stderr, "Failed to open file from structure.\n");
            }
            args.logfd = logfd;
            args.b_option = true;
            long b_size;
            b_size = strtol(argv[4], &ptr, 10);
            args.buffer_size = b_size;
        } else {
            args.p_option = false;
            args.b_option = false;
            args.buffer_size = -1;
        }
    } 
    printf("args.buffer_size = %ld\n", args.buffer_size);

    //make a "dummy" first node
    struct node * shared_queue;
    shared_queue = create_dummy(shared_queue);
    
    //initialize the struct variables
    args.c_thread_no = 0;
    args.num_working = 1;

    //args.node_count = 0;
    args.filepath = argv[2];
    args.head = shared_queue;
    args.queue_cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    args.queue_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    args.cons_num = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_cond_init(args.queue_cond, NULL);
	pthread_mutex_init(args.queue_mutex, NULL);
    args.hist_cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    args.hist_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_cond_init(args.hist_cond, NULL);
	pthread_mutex_init(args.hist_mutex, NULL);
    args.myargv_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(args.myargv_mutex, NULL);
    pthread_mutex_init(args.cons_num, NULL);
    for(i = 0; i < 26; i++) {
        args.histogram[i] = 0;
    }

    //make producer thread
    error = pthread_create(&prod, NULL, producer, (void *) &args);
    if (error != 0) {
        printf("Error creating producer thread\n");   
    }

    //make consumer thread(s)
    for (i = 0; i < num_threads; i++) {
        //c_args[i] = args;
        error = pthread_create(&cons[i], NULL, consumer, (void *) &args);
        if (error != 0) {
            printf("Error creating consumer thread %d\n", i);   
        }
        
    }

    //wait for ALL threads to finish
    pthread_join(prod, NULL);
    for (i = 0; i < num_threads; i++) {
        pthread_join(cons[i], NULL);
    }

    //used to check the output in terminal
    free_list(args.head);
    for(i = 0; i < 26; i++) {
        printf("args.histogram[%d] = %d\n", i, args.histogram[i]);
    }

    if ((rfd = fopen("result.txt", "w")) == NULL) {
        fprintf(stderr, "Failed to open file from structure.\n");
    }

    for (i = 0; i < 26; i++) {
        fprintf(rfd, "%c: %d\n", 'a' + i, args.histogram[i]);
    }

    free(args.queue_cond);
    free(args.queue_mutex);
    free(args.hist_cond);
    free(args.hist_mutex);
    free(args.myargv_mutex);
    if (args.p_option == true)
        fclose(logfd);
    fclose(rfd);

    // These may or may not be needed
    // if (error = pthread_mutex_destroy(&queue_lock))
    //     fprintf(stderr, "Failed to destroy queue_lock:%s\n", strerror(error));

    // if (error = pthread_mutex_destroy(&hist_lock))
    //     fprintf(stderr, "Failed to destroy hist_lock:%s\n", strerror(error));

}