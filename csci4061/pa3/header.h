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
#include <stdbool.h>
#include <errno.h>

void main(int argc, char *argv[]);
void *producer(void *nargs);
void *consumer(void *nargs);

struct node {
    char *data;
    int line_number;
    struct node *next;
};

// Argument structure for threads
typedef struct argstruct {
    int c_thread_no;
    bool p_option;
    bool b_option;
    bool bp_option;
    long buffer_size;
    FILE *logfd;
    //int node_count;
    int num_working; //needs to be a pointer
    char *filepath;
    struct node* head;
    //struct node* tail;
    int histogram[26]; //needs to be a pointer
    //struct char_histogram* histogram;
    pthread_mutex_t* queue_mutex;
    pthread_cond_t* queue_cond;
    pthread_mutex_t* hist_mutex;
    pthread_cond_t* hist_cond;
    pthread_mutex_t* myargv_mutex;
    pthread_mutex_t *cons_num;
} args_t;

int makeargv(const char *s, const char *delimiters, char ***argvp);
void freemakeargv(char ***argv);
struct node* create_dummy(struct node* head);
struct node* create_node(struct node* node, char *data);
void append_node(struct node* head, char* data, int line_number);
struct node* remove_2nd_node(struct node* head);
void print_list(struct node *head);
void free_list(struct node* head);



#endif
