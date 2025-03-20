#include <stdio.h>
#include <assert.h>
#include <string.h>

#include<stdint.h>
#include "shell.h"

extern CPU_State CURRENT_STATE;

/*
se puede modelar como punteros a función. 
no importa el orden en el que se empieza a comparar. 
pasar parámetros a main. 
*/


void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
    * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
    * access memory. 
    * */

    uint64_t PC = CURRENT_STATE.PC;
    uint32_t instruction =  mem_read_32(PC);

    //Para chequear lo que estamos leyendo
    printf("Instruction: %x\n", instruction);

    int adds_extended_opcode = 0b10101011001<<24;
    int adds_extended_mask = 0b11111111111<<24;
    int adds_immediate_opcode = 0b10110001<<24;
    int adds_immediate_mask = 0b11111111<<24;


    printf("opcode: %x\n", adds_immediate_opcode);

    // printf("Mask: %n", instruction & adds_mask);

    if ((instruction & adds_extended_mask) == adds_extended_opcode){
        //imm12
        int imm12_mask = 0b111111111111<<10;
        int imm12 = (instruction & imm12_mask)>>10;
        //Rn
        int Rn_mask = 0b11111<<5>>20;
    }

    if ((instruction & adds_immediate_mask) == adds_immediate_opcode) {
        printf("es un ADDS immediate!!!!\n");
    } else {
        printf("no es un ADDS immediate\n");
    }

}