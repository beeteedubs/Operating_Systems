#include <stdlib.h>

typdef struct Node{
	int *data;
	node *next;
}node;

void push(node** head, int *new_data, size_t data_size){
	// allocate mem for new node
	node new_node = (node*)malloc(sizeof(node));

	new_node->data = malloc(data_size);
	new_node->next = *head;

	// copy contents to newly allocated memory
	*new_node.data = *new_data;

	// add to end of list
	while *head

	return;
}

int main(){
	
	return 0;
}
