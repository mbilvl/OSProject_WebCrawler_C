#include "hashqueue.h"

//function to create a list node
node_t *create_new_list_node(char *url){

	node_t *node = malloc(sizeof(node_t));
	strncpy(node->url, url, URL_LEN - 1);
	node->url[URL_LEN - 1] = '\0';  // Ensure null termination
	node->next = NULL;

	return node;
}
//function to insert node at head
void insert_node_head(node_t **head, char* url){

	node_t *tmp;
	tmp = create_new_list_node(url); 
	tmp->next = *head;
	*head = tmp;
}
