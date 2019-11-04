/*test machine: CSELAB_machine_name * date: mm/dd/yy
* name: full_name1 , [full_name2]
* x500: id_for_first_name , [id_for_second_name] */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
// pthread.h included in header.h

// implement shared queue, final histogram here..

struct node {
    char *data;
    struct node *next;
};

struct node* create_dummy(struct node* head) {
    head = (struct node*)malloc(sizeof(struct node));
    head->data = "dummy\n";
    head->next = NULL;

    return head;
}

struct node* create_node(struct node* node, char *data) {
    //printf("Entered create_node__");
    node = (struct node*)malloc(sizeof(struct node));
    node->data = data;
    //printf("node->data: %s", node->data);
    node->next = NULL;
    //printf("Exiting create_node\n");
    return node;
}

// This be broken. While it is appending a node, it's also overwriting
// the data in every node. I don't understand why.
void append_node(struct node* head, struct node* new_node) {
    //printf("Entered append_node__\n");
    //printf("head->data: %s", head->data);
    //printf("node->data: %s", new_node->data);
    //node = (struct node*)malloc(sizeof(struct node));
    // struct node * temp = (struct node*)malloc(sizeof(struct node));
    // struct node * tail = (struct node*)malloc(sizeof(struct node));
    struct node* temp = head;
    //printf("temp->data: %s", temp->data);
    while (temp->next != NULL) {
        printf("Befor, temp->data: %s", temp->data);
        temp = temp->next;
        printf("After, temp->data: %s", temp->data);
    }
    temp->next = new_node;
    //printf("After, temp->data: %s", temp->data);
    //printf("After, temp->next->data: %s", temp->next->data);
    //printf("node->data: %s", new_node->data);
    //printf("Exiting append_node\n");
};

struct node* remove_2nd_node(struct node* head) {
    struct node * temp; // = (struct node*)malloc(sizeof(struct node));
    // temp = head;
    if (head->next != NULL) {
        temp = head->next;
        head->next = head->next->next;
    }
    free(temp);
    return head;
};

void print_list(struct node* head){
    struct node* cur = head;
    int count = 1;
    while(cur != NULL){
        printf("node %d: %s", count, cur->data);
        cur = cur->next;
        count++;
    }
    printf("\n");
}