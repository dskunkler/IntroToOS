/*test machine: CSELAB_machine_name * date: mm/dd/yy
* name: full_name1 , [full_name2]
* x500: id_for_first_name , [id_for_second_name] */

/*
header.h, header for all source files
it will: 
- structure definition
- global variable, lock declaration (extern)
- function declarations
*/

#ifndef _HEADER_H_
#define _HEADER_H_

#define LINE_SIZE 1024

// header here..
#include <pthread.h>

struct node {
    char *data;
    struct node *next;
};

void main(int argc, char *argv[]);
void * producer(void * nargs);
void * consumer(void * nargs);

// Argument structure for threads
typedef struct argstruct {
    char *filepath;
    struct node *head;
    struct node *tail;
    int num_nodes;
    struct linked_list* list;
    struct char_histogram* histogram;
    struct pthread_mutex_t* queue_lock;
    struct pthread_mutex_t* hist_lock;
} args_t;

void create_dummy(struct node** head);

void create_node(struct node** node, char *data);

void append_node(struct node** tail, struct node* node);

void remove_2nd_node(struct node** head);

void print_list(struct node** head);

void create_smarty(struct node** head);

// Global result histogram goes here
// typedef struct char_histogram {

// };

//





#endif
