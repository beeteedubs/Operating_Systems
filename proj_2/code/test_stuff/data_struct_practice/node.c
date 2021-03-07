#include <stdlib.h>
#include <stdio.h>

struct Node{
	int data;
	struct Node *next;
};

void append(struct Node** head, int new_data){
	// malloc node
	struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
	
	// put in data
	new_node->data = new_data;

	// last node, so make next NULL
	new_node->next = NULL;

	// if LL empty, assign as head
	if (*head == NULL){
		*head = new_node;
		return;
	}
	
	// traverse until last node
	struct Node *last = *head; // start last at head to traverse

	while (last->next != NULL)
		last = last->next;

	last->next = new_node;
	return;
}

struct Node* deleteFirst(struct Node** head){
	
	// create temp reference to head
	struct Node *temp = *head;
	
	// check if empty LL 
	if (temp != NULL){
		// move head 1 node down
		*head = temp->next;
	}
	else{
		puts("\n This list was empty \n");
	}
	return temp;
}

void printList(struct Node *node){
	puts("\n");
	while (node != NULL){
		printf(" %d ", node->data);
		node = node->next;
	}
	puts("\n");
}

int main(){
	// start w/ empty LL
	struct Node* head = NULL;
	
	append(&head, 3);
	append(&head, 2);
	append(&head, 1);

	printList(head);

	puts("delete 3");
	deleteFirst(&head);
	printList(head);
	return 0;
}
