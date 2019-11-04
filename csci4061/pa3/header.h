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

void main(int argc, char *argv[]);
void * producer(void * nargs);
void * consumer(void * nargs);

// Argument structure for threads
typedef struct argstruct {
    char *filepath;
    struct node* head;
    struct node* tail;
    struct linked_list* list;
    struct char_histogram* histogram;
    struct pthread_mutex_t* queue_lock;
    struct pthread_mutex_t* hist_lock;
} args_t;

// linked-list structure definition goes here
// struct linked_list {
//     struct node * dummy_head;

// };

struct node* create_dummy(struct node* head);

struct node* create_node(struct node* node, char *data);

void append_node(struct node* head, struct node* node);

struct node* remove_2nd_node(struct node* head);

void print_list(struct node* head);

// Global result histogram goes here
// typedef struct char_histogram {

// };

//





#endif
