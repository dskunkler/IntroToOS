/*test machine: CSELAB_machine_name * date: mm/dd/yy
* name: full_name1 , [full_name2]
* x500: id_for_first_name , [id_for_second_name] */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "header.h"
// pthread.h included in header.h

// Directive:
// 1)   Read from the shared queue
// 2)   Compute the "word count statisticts"
// 3)   synchronize data with the Result Histogram


// if (pthread_mutex_lock(args->queue_mutex) != 0) 
//     fprintf(stderr, "Failed to lock queue_lock.\n");
// if (pthread_mutex_unlock(args->queue_mutex) != 0)
//     fprintf(stderr, "Failed to unlock queue_lock.\n");

// consumer thread function
void *consumer(void * nargs) 
{
    args_t * args = (args_t*) nargs;
    //delim needs to be everything but a-zA-Z.
    char delim[] = ",.?!/:;<>@#$^&*()-_=+[]{}| \t\n\\\"\'1234567890";
    char delim2[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
    int numTokens;

    
    if (args->p_option == true) {
        pthread_mutex_lock(args->cons_num);
        fprintf(args->logfd, "consumer %d\n", args->c_thread_no);
        args->c_thread_no++;
        pthread_mutex_unlock(args->cons_num);
    }    
    

    //printf("args->c_thread_no is %d\n", args->c_thread_no);
    while (1) { 
        int charCount[26] = {0};
        char **myargv; 
        
        //pthread_mutex_lock(args->queue_mutex);
        //printf("args->node_count = %d", args->node_count);
        // if (args->node_count == 0 && args->num_working == 0) {
        //     printf("1st check: about to exit a consumer thread (%d)\n", args->num_working);
        //     pthread_mutex_unlock(args->queue_mutex);
        //     pthread_exit(0);
        // }
        // pthread_mutex_unlock(args->queue_mutex);

        pthread_mutex_lock(args->queue_mutex);
        while (args->head->next == NULL) {
            if (args->num_working == 0) {
                if (args->p_option == true) {
                    fprintf(args->logfd, "consumer %ld: -1\n", pthread_self());
                }
                printf("about to exit a consumer thread %ld\n", pthread_self());
                pthread_cond_signal(args->queue_cond);
                pthread_mutex_unlock(args->queue_mutex);
                pthread_exit(0);
            }

            printf("wait condition on thread %ld\n", pthread_self()); 
            pthread_cond_wait(args->queue_cond, args->queue_mutex);
            //printf("wait condition end on thread %ld\n", pthread_self());
        }

        struct node * new_node = remove_2nd_node(args->head); //CRITICAL CODE
        //args->node_count--; //CRITICAL CODE
        pthread_mutex_unlock(args->queue_mutex);

        // int line_len;
        // line_len = strlen(new_node->data);
        // printf("len: %d\n", line_len);
        // char *line;
        // line = (char*)malloc(strlen(new_node->data));
        // strcpy(line, new_node->data);
        // printf("line: %s", line);
        // int delim_len;
        // delim_len = strlen(delim);
        // int count = 0;

        // for (int i = 0; i < line_len; i++) {
        //     for (int j = 0; j < delim_len; j++) {
        //         if (line[i] == delim2[j]) {

        //             //increment count for line[i] letter
        //             count++;
        //             printf("delimiter found \"%c\"", delim[j]);   
        //         } else {}
        //     }
        //     count = 1;
        // }


        pthread_mutex_lock(args->myargv_mutex);
        numTokens = makeargv(new_node->data, delim, &myargv);
        //printf("numTokens is %d\n", numTokens);
        for (int i = 0; i < numTokens; i++) {

            char *str;
            int len;
            len = strlen(myargv[i]);
            str = (char*)malloc(len);
            strcpy(str, myargv[i]);
            //printf("strlen(myargv[%d]) is \"%d\" str: \"%s\" \n", i, len, str);
            charCount[toupper(str[0]) - 'A'] += 1;
        }
        pthread_mutex_unlock(args->myargv_mutex);

        if (args->p_option == true) {
            fprintf(args->logfd, "consumer %ld: %d\n", pthread_self(), new_node->line_number);
        }

        //freemakeargv(&myargv);
        free(new_node);

        pthread_mutex_lock(args->hist_mutex);
        for (int i = 0; i < 26; i++) {
            args->histogram[i] += charCount[i];
        }
        pthread_mutex_unlock(args->hist_mutex);
    }
}
