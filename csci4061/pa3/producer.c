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

// needs the filepath and the shared queue structure
void * producer(void * nargs) 
{
    printf("Entered Producer Thread\n");
    args_t * args = (args_t*) nargs;
    FILE *myfd;
    char textline[1024];
    char *line;
    char *filepath;
    int error;

    printf("args.filepath: %s\n", args->filepath);
    filepath = args->filepath;
    printf("filepath[0]: %s\n", filepath);

    
    if ((myfd = fopen(filepath, "r")) == NULL) {
        fprintf(stderr, "Failed to open file from structure.\n");
    }

    struct node * next_node;
    while (fgets(textline, 1024, myfd)) {
        line = (char*)malloc(sizeof(textline));
        strncpy(line,textline,strlen(textline));      
        create_node(&next_node, line);
        //lock mutex to add to linked list
        pthread_mutex_lock(args->queue_lock);    
        append_node(&args->tail, next_node);
        args->num_nodes++;
        //broadcast number of nodes
        pthread_cond_broadcast(args->cond);
        pthread_mutex_unlock(args->queue_lock);
    }

    //end of file, add -1
    pthread_mutex_lock(args->queue_lock);
    create_node(&next_node, "-1");
    append_node(&args->tail, next_node);
    args->num_nodes++;
    pthread_cond_broadcast(args->cond);
    pthread_mutex_unlock(args->queue_lock);

    print_list(&args->head);

    // if (pthread_mutex_unlock(&args->queue_lock) != 0) {
    //     fprintf(stderr, "Failed to unlock queue_lock.\n");
    // }


}
