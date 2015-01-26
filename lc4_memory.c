#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc4_memory.h"
/*
struct row_of_memory {
	short unsigned int address ;
	char * label ;
	short unsigned int contents ;
	char * assembly ;
	struct row_of_memory *next ;
} ;
*/

struct row_of_memory* add_to_list    (struct row_of_memory* head, 
				      short unsigned int address, 
				      short unsigned int contents) 
{
	/* takes in head pointer to a linked list: head
	   creates a new node with address & contents passed in
	   adds node to end of linked list pointed to by head
	   if list is empty, new node is now head
	   always returns head pointer */
	return NULL; //not implemented or used

}

struct row_of_memory* insert_to_list (struct row_of_memory** head, 
				      short unsigned int address, 
				      short unsigned int contents) 
{
	/* insert a node to the list in increasing address order, return the new node added*/
	struct row_of_memory* temp = NULL;
	struct row_of_memory* new_node = NULL;
	struct row_of_memory* temp2 = NULL;
	
	temp = *head;
	if(*head != NULL) {
		while(temp->next != NULL && temp->next->address <= address) {
			temp = temp->next;
		}
		if(temp->address == address) { //node already created somehow! just update it and return
			if(contents != 0) {
				temp->contents = contents;
			}
			return temp;
		}
	}
	
	/* allocate memory for a single node */
	new_node = malloc (sizeof (struct row_of_memory));
	if(new_node == NULL) {
		delete_list(*head);
		return NULL;
	}
	/* populate fields in newly allocated node w/ address&contents */
	new_node->address = address;
	new_node->contents = contents;
	new_node->label = NULL;
	new_node->assembly = NULL;
	new_node->next = NULL;
	
	if (*head == NULL) {
		*head = new_node;
	} else {
		temp2 = temp->next;
		temp->next = new_node;
		new_node->next = temp2;
	}
	return new_node; 
}

struct row_of_memory* search_address (struct row_of_memory* head, 
				      short unsigned int address ) 
{
	/* traverse linked list, searching each node for "address"  */
	struct row_of_memory* temp = NULL;

	for(temp = head; temp != NULL; temp = temp->next) {
		if(temp->address == address) {
			break;
		}
	}

	return temp;
}


struct row_of_memory* search_opcode  (struct row_of_memory* head, 
				      short unsigned int opcode) 
{
	/* traverse linked list until node is found with matching opcode
	   AND "assembly" field of node is empty */
	struct row_of_memory* temp = NULL;
	for(temp = head; temp != NULL; temp = temp->next) {
		short unsigned int code = temp->contents;
		if((code >> 12) == opcode && temp->assembly == NULL) {
			break;
		}
		
	}
	/* return pointer to node in list that matches */
	/* return NULL if list is empty or if no matching nodes */
	return temp;
}

void print_list (struct row_of_memory* head ) 
{
	/* make sure head isn't NULL */
	/* print out a header */
	/* traverse linked list, print contents of each node */	
	struct row_of_memory* temp = NULL;
	if(head == NULL) {
		printf("Empty list.\n");
		return;
	}
	printf("%-16s%-12s%-14s%-20s\n", "<label>", "<address>", "<contents>", "<assembly>");
	for(temp = head; temp != NULL; temp = temp->next) {
		//print each line, if string is null, print "" instead of "(null)"
		printf("%-16s  %04X        %04X %s\n", (temp->label == NULL) ? "" : temp->label, temp->address, temp->contents, (temp->assembly == NULL) ? "" : temp->assembly); 
	}
	return ;
}

struct row_of_memory* delete_list (struct row_of_memory* head ) 
{
	/* delete entire list node by node */
	struct row_of_memory* temp = NULL;
	for(temp = head; temp != NULL; temp = head) {
		head = temp->next;
		free(temp->label);
		free(temp->assembly);
		free(temp);
	}
	return NULL ;
}

