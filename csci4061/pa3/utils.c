/*test machine: CSELAB_machine_name * date: mm/dd/yy
* name: full_name1 , [full_name2]
* x500: id_for_first_name , [id_for_second_name] */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
// pthread.h included in header.h

// implement shared queue, final histogram here..

int makeargv(const char *s, const char *delimiters, char ***argvp)
{
	int error;
	int i;
	int numtokens;
	const char *snew;
	char *t;

	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
		errno = EINVAL;
		return -1;
	}
	*argvp = NULL;
	snew = s + strspn(s, delimiters);         /* snew is real start of string */
	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);
	numtokens = 0;
	if (strtok(t, delimiters) != NULL)     /* count the number of tokens in s */
		for(numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++);
		/* create argument array for ptrs to the tokens */
	if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL){
		error = errno;
		free(t);
		errno = error;
		return -1;
	}
	/* insert pointers to tokens into the argument array */
	if (numtokens == 0)
		free(t);
	else {
		strcpy(t, snew);
		**argvp = strtok(t, delimiters);
		for (i = 1; i < numtokens; i++)
		*((*argvp) + i) = strtok(NULL, delimiters);
	}
	*((*argvp) + numtokens) = NULL;            /* put in final NULL pointer */
	return numtokens;
}


void freemakeargv(char ***argv) 
{
    if (argv == NULL)
       return;
    if (*argv != NULL)
       free(*argv);
    free(argv);
}


struct node* create_dummy(struct node* head) 
{
    head = (struct node*)malloc(sizeof(struct node));
    head->data = "dummy\n";
    head->line_number = 0;
    head->next = NULL;

    return head;
}


void print_list(struct node* head)
{
    struct node* cur = head;
    int count = 1;
    while(cur != NULL){
        printf("node %d: %s", cur->line_number, cur->data);
        cur = cur->next;
        count++;
    }
    printf("\n");

    cur = head;
    while(cur != NULL){
        cur = cur->next;
        free(head);
        head = cur;
        count--;
    }
}

void free_list(struct node* head)
{
    struct node* cur = head;
    cur = head;
    while(cur != NULL){
        cur = cur->next;
        free(head);
        head = cur;
    }
}


void append_node(struct node* head, char * data, int line_number) 
{
    struct node * new_node = (struct node*)malloc(sizeof(struct node));
    new_node->data = data;
    new_node->line_number = line_number;
    new_node->next = NULL;

    struct node * temp;
    temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_node;
};


struct node* remove_2nd_node(struct node* head) 
{
    struct node * temp; 
    if (head->next != NULL) {
        temp = head->next;
        head->next = head->next->next;
    }
    return temp;
};

