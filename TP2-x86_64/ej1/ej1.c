#include "ej1.h"

string_proc_list* string_proc_list_create(void){
	string_proc_list* list = (string_proc_list*) malloc(sizeof(string_proc_list));
	if(list == NULL){
		fprintf(stderr, "Error: No se pudo crear la lista\n");
		exit(EXIT_FAILURE);
	}
	list->first = NULL;
	list->last  = NULL;
	return list;
}

string_proc_node* string_proc_node_create(uint8_t type, char* hash){
	string_proc_node* node = (string_proc_node*) malloc(sizeof(string_proc_node)); 
	if(node == NULL){
		fprintf(stderr, "Error: No se pudo crear el nodo\n");
		exit(EXIT_FAILURE);
	}
	node->next      = NULL;
	node->previous  = NULL;
	node->hash	    = hash;
	node->type      = type;
	return node;
}

void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash){
	string_proc_node* new_node = string_proc_node_create(type, hash);
	new_node -> previous = list -> last;
	(list -> last) -> next = new_node;
	list -> last = new_node;
}

char* string_proc_list_concat(string_proc_list* list, uint8_t type , char* hash){
	string_proc_node *node = list -> first;
	char *result;
	while (node != NULL) {
		if (node -> type == type) {
			result = str_concat(result, node -> hash);
		}
	}
	return result;
}


/** AUX FUNCTIONS **/

void string_proc_list_destroy(string_proc_list* list){

	/* borro los nodos: */
	string_proc_node* current_node	= list->first;
	string_proc_node* next_node		= NULL;
	while(current_node != NULL){
		next_node = current_node->next;
		string_proc_node_destroy(current_node);
		current_node	= next_node;
	}
	/*borro la lista:*/
	list->first = NULL;
	list->last  = NULL;
	free(list);
}
void string_proc_node_destroy(string_proc_node* node){
	node->next      = NULL;
	node->previous	= NULL;
	node->hash		= NULL;
	node->type      = 0;			
	free(node);
}


char* str_concat(char* a, char* b) {
	int len1 = strlen(a);
    int len2 = strlen(b);
	int totalLength = len1 + len2;
    char *result = (char *)malloc(totalLength + 1); 
    strcpy(result, a);
    strcat(result, b);
    return result;  
}

void string_proc_list_print(string_proc_list* list, FILE* file){
        uint32_t length = 0;
        string_proc_node* current_node  = list->first;
        while(current_node != NULL){
                length++;
                current_node = current_node->next;
        }
        fprintf( file, "List length: %d\n", length );
		current_node    = list->first;
        while(current_node != NULL){
                fprintf(file, "\tnode hash: %s | type: %d\n", current_node->hash, current_node->type);
                current_node = current_node->next;
        }
}

