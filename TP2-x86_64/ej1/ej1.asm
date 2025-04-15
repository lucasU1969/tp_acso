; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0
%define string_proc_list_size 16
%define string_proc_node_size 32
%define first_offset 0
%define last_offset 8
%define next_offset 0
%define previous_offset 8
%define type_offset 16
%define hash_offset 24

section .data

section .text
global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat
extern strlen
extern memcpy
extern strcpy
extern strcat

; string_proc_list* string_proc_list_create_asm(void)
string_proc_list_create_asm:
    ; Prólogo
    push rbp
    mov rbp, rsp
    
    ; Llamar a malloc para crear la estructura de la lista
    mov rdi, string_proc_list_size   ; Tamaño de la estructura
    call malloc
    
    ; Comprobar si malloc tuvo éxito
    test rax, rax
    jz .error
    
    ; Inicializar first y last a NULL
    mov qword [rax + first_offset], NULL
    mov qword [rax + last_offset], NULL
    
    ; Epílogo
    mov rsp, rbp
    pop rbp
    ret
    
.error:
    ; En caso de error, retornar NULL
    mov rax, NULL
    mov rsp, rbp
    pop rbp
    ret

; string_proc_node* string_proc_node_create_asm(uint8_t type, char* hash)
string_proc_node_create_asm:
    ; Prólogo
    push rbp
    mov rbp, rsp
    
    ; Guardar los argumentos
    push rdi    ; type
    push rsi    ; hash
    
    ; Llamar a malloc para crear el nodo
    mov rdi, string_proc_node_size
    call malloc
    
    ; Restaurar los argumentos
    pop rsi     ; hash
    pop rdi     ; type
    
    ; Comprobar si malloc tuvo éxito
    test rax, rax
    jz .error
    
    ; Inicializar el nodo
    mov qword [rax + next_offset], NULL    ; node->next = NULL
    mov qword [rax + previous_offset], NULL ; node->previous = NULL
    mov qword [rax + hash_offset], rsi     ; node->hash = hash
    mov byte [rax + type_offset], dil      ; node->type = type
    
    ; Epílogo
    mov rsp, rbp
    pop rbp
    ret
    
.error:
    ; En caso de error, retornar NULL
    mov rax, NULL
    mov rsp, rbp
    pop rbp
    ret

; void string_proc_list_add_node_asm(string_proc_list* list, uint8_t type, char* hash)
string_proc_list_add_node_asm:
    ; Prólogo
    push rbp
    mov rbp, rsp
    
    ; Guardar los registros que usaremos
    push rbx
    push r12
    push r13
    push r14
    
    ; Guardar los argumentos
    mov rbx, rdi    ; list
    mov r12b, sil   ; type
    mov r13, rdx    ; hash
    
    ; Verificar si list es NULL
    test rbx, rbx
    jz .end
    
    ; Verificar si hash es NULL
    test r13, r13
    jz .end
    
    ; Crear un nuevo nodo
    movzx rdi, r12b   ; type (con extensión de ceros)
    mov rsi, r13      ; hash
    call string_proc_node_create_asm
    mov r14, rax      ; Guardar el nuevo nodo en r14
    
    ; Verificar si la creación del nodo tuvo éxito
    test r14, r14
    jz .end
    
    ; Verificar si la lista está vacía (list->last == NULL)
    mov rax, [rbx + last_offset]
    test rax, rax
    jnz .list_not_empty
    
    ; Si la lista está vacía, first y last apuntan al nuevo nodo
    mov [rbx + first_offset], r14
    mov [rbx + last_offset], r14
    jmp .end
    
.list_not_empty:
    ; La lista no está vacía, agregar al final
    mov rcx, [rbx + last_offset]    ; Obtener el último nodo actual
    
    ; last->next = new_node
    mov [rcx + next_offset], r14
    
    ; new_node->previous = last
    mov [r14 + previous_offset], rcx
    
    ; list->last = new_node
    mov [rbx + last_offset], r14
    
.end:
    ; Epílogo
    pop r14
    pop r13
    pop r12
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

; char* string_proc_list_concat_asm(string_proc_list* list, uint8_t type, char* hash)
string_proc_list_concat_asm:
    ; Prólogo
    push rbp
    mov rbp, rsp
    
    ; Guardar los registros que usaremos
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, 8      ; Alinear pila a 16 bytes
    
    ; Guardar los argumentos
    mov rbx, rdi    ; list
    mov r12b, sil   ; type
    mov r13, rdx    ; hash
    
    ; Verificar si list es NULL
    test rbx, rbx
    jz .return_null
    
    ; Verificar si hash es NULL
    test r13, r13
    jz .return_null
    
    ; Preparar para duplicar el hash inicial
    mov rdi, r13
    call strlen
    
    ; Longitud + 1 para el terminador nulo
    lea rdi, [rax + 1]
    call malloc
    mov r14, rax    ; r14 = result
    
    ; Verificar si malloc tuvo éxito
    test r14, r14
    jz .return_null
    
    ; Copiar el hash inicial a result
    mov rdi, r14
    mov rsi, r13
    call strcpy
    
    ; Inicializar node = list->first
    mov r15, [rbx + first_offset]
    
    ; Bucle para recorrer los nodos
.loop:
    ; Verificar si hemos llegado al final de la lista
    test r15, r15
    jz .end_loop
    
    ; Verificar si el tipo coincide
    movzx eax, byte [r15 + type_offset]
    cmp al, r12b
    jne .next_node
    
    ; El tipo coincide, concatenar
    mov rdi, r14                     ; result
    mov rsi, [r15 + hash_offset]     ; node->hash
    call str_concat
    
    ; Liberar el result anterior
    mov rdi, r14
    push rax                         ; Guardar el nuevo result
    call free
    pop r14                          ; Restaurar el nuevo result en r14
    
.next_node:
    ; Avanzar al siguiente nodo
    mov r15, [r15 + next_offset]
    jmp .loop
    
.end_loop:
    ; Devolver el resultado
    mov rax, r14
    jmp .cleanup
    
.return_null:
    mov rax, NULL
    
.cleanup:
    ; Epílogo
    add rsp, 8      ; Deshacer alineamiento
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    mov rsp, rbp
    pop rbp
    ret