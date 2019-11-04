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
    char *filepath;
    int error;

    printf("args.filepath: %s\n", args->filepath);
    filepath = args->filepath;
    printf("filepath[0]: %s\n", filepath);

    

    // if (pthread_mutex_lock(&args->queue_lock) != 0) {
    //     fprintf(stderr, "Failed to lock queue_lock.\n");
    // }

    if ((myfd = fopen(filepath, "r")) == NULL) {
        fprintf(stderr, "Failed to open file from structure.\n");
    }

    while (fgets(textline, 1024, myfd)) {
        printf("textline: %s", textline);
        struct node * next_node;
        //next_node = {textline, NULL};
        next_node = create_node(next_node, textline);
        append_node(args->head, next_node);
    }

    print_list(args->head);

    // if (pthread_mutex_unlock(&args->queue_lock) != 0) {
    //     fprintf(stderr, "Failed to unlock queue_lock.\n");
    // }


}
