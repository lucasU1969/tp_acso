#include "ej1.h"

string_proc_list* string_proc_list_create(void){
	// Inicializa una estructura de lista.
	string_proc_list* list = (string_proc_list*) malloc(sizeof(string_proc_list));
	if(list == NULL){
		return NULL;
	}
	list->first = NULL;
	list->last  = NULL;
	return list;
}


string_proc_node* string_proc_node_create(uint8_t type, char* hash){
	// Inicializa un nodo con el tipo y el hash dado.
	string_proc_node* node = (string_proc_node*) malloc(sizeof(string_proc_node));
	if(node == NULL){
		return NULL;
	}
	node->next = NULL;
	node->previous = NULL;
	node->hash = hash;
	node->type = type;
	return node;
}


void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash){
	// Agrega un nodo nuevo al final de la lista con el tipo y el hash dado.
	if (list == NULL) {
		return;
	}
	if (hash == NULL) {
		return;
	}
	string_proc_node* new_node = string_proc_node_create(type, hash);
	if (list->last == NULL) {
		list->first = new_node;
		list->last = new_node;
	} else {
		list->last->next = new_node;
		new_node->previous = list->last;
		list->last = new_node;
	}
}

char* my_strdup(const char* str) {
    size_t len = strlen(str) + 1;  // +1 para el terminador nulo
    char* copy = (char*) malloc(len);
    if (copy == NULL) {
		return NULL;
    }
    memcpy(copy, str, len);
    return copy;
}


char* string_proc_list_concat(string_proc_list* list, uint8_t type, char* hash){
	// Genera un nuevo hash concatenando el pasado por parámetro con todos los hashes
	// de los nodos de la lista cuyos tipos coinciden con el pasado por parámetro.
	if (list == NULL) {
		return NULL;
	}
	if (hash == NULL) {
		return NULL;
	}
	string_proc_node *node = list->first;
	char *result = my_strdup(hash);
	if (result == NULL) {
		return NULL;
	}
	while (node != NULL) {
		if (node->type == type) {
			char* temp = str_concat(result, node->hash);
			free(result);
			result = temp;
		}
		node = node->next;
	}
	return result;
}



/** AUX FUNCTIONS **/

void string_proc_list_destroy(string_proc_list* list){

	/* borro los nodos: */
	if (list == NULL) return;
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
	if (node == NULL) return;
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

