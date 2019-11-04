/*test machine: CSELAB_machine_name * date: mm/dd/yy
* name: full_name1 , [full_name2]
* x500: id_for_first_name , [id_for_second_name] */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
// pthread.h included in header.h

// implement shared queue, final histogram here..


void create_dummy(struct node** head) {
    (*head) = (struct node*)malloc(sizeof(struct node));
    (*head)->data = "dummy\n";
    (*head)->next = NULL;

}


void create_node(struct node** node, char *data) {
    //printf("Entered create_node__");
    (*node) = (struct node*)malloc(sizeof(struct node));
    (*node)->data = data;
    (*node)->next = NULL;
    
}

//Add the new node to the tail
void append_node(struct node **tail, struct node* new_node){
   
    (*tail)->next = new_node;
    *tail = (*tail)->next;
};

void remove_2nd_node(struct node** head) {
    struct node * temp; // = (struct node*)malloc(sizeof(struct node));
    // temp = head;
    if ((*head)->next != NULL) {
        temp = (*head)->next;
        (*head)->next = (*head)->next->next;
    }
    free(temp);
};

void print_list(struct node **head){
    printf("HELLO FROM PRINT LIST\n");
    struct node* cur = (*head);
    int count = 1;
    while(cur != NULL){
        printf("node %d: %s", count, cur->data);
        cur = cur->next;
        count++;
    }
    printf("\n");
}