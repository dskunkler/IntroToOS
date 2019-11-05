/*test machine: CSELAB_machine_name * date: mm/dd/yy
* name: full_name1 , [full_name2]
* x500: id_for_first_name , [id_for_second_name] */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
// pthread.h included in header.h

// Directive:
// 1)   Read from the shared queue
// 2)   Compute the "word count statisticts"
// 3)   synchronize data with the Result Histogram

// needs the shared queue structure and the result histogram structure
void * consumer(void * nargs) 
{
    printf("Entered Consumer Thread\n");
    args_t * args = (args_t*) nargs;

    pthread_mutex_lock(args->queue_lock);
    while(args->num_nodes < 1){
        pthread_cond_wait(args->cond, args->queue_lock);
    }

    //return NULL;
}
