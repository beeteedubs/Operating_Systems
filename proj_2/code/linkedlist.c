#include <stdio.h>
#include <stdlib.h>

struct Node{
	int data;
	struct Node *next;
};

void printList(struct Node *head){
	// get head.next 
	while(head->next != NULL){
		printf(" %d ", head->data);
	}
	return;
}

int main(){
	return 0;
}
