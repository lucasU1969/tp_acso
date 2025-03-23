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

void subs_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void subs_extended_register(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void update_flags(uint64_t result, CPU_State *NEXT_STATE);


void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
    * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
    * access memory. 
    * */

    uint64_t PC = CURRENT_STATE.PC;
    uint32_t instruction =  mem_read_32(PC);

    printf("Instruction: %x\n", instruction);

//  Masks
    int mask_11bits = 0b11111111111<<21;
    int mask_8bits = 0b11111111<<24;


//  Opcodes
    int subs_immediate_opcode = 0b11110001<<24;
    int subs_extended_register_opcode = 0b11101011001<<21;



    if ((instruction & mask_8bits) == subs_immediate_opcode){
        subs_immediate(instruction, &CURRENT_STATE, &NEXT_STATE); 
    }

    if ((instruction & mask_11bits) == subs_extended_register_opcode){
        subs_immediate(instruction, &CURRENT_STATE, &NEXT_STATE);
    }



    NEXT_STATE.PC  = PC + 4;
}


void subs_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
//  se puede hacer en términos de un add a un not immediate. 

    char shift = (pars & 0b11<<22) >> 22;
    short imm12 = (pars & 0b111111111111<<10)>>10;
    if (shift) {
        imm12 = imm12 << 12;
    }
    short Rn = (pars & 0b11111<<5)>>5;
    short Rd = pars & 0b11111;

    NEXT_STATE->REGS[Rd] = (CURRENT_STATE->REGS[Rn]) - imm12;

    update_flags(NEXT_STATE->REGS[Rd], NEXT_STATE);
}


void subs_extended_register(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
    short Rm = (pars & 0b11111<<16)>>16;
    short option = (pars & 0b111<<13)>>13; 
    short imm3 = (pars & 0b111>>10)>>10;
    short Rn = (pars & 0b11111<<5)>>5;
    short Rd = pars & 0b11111;
    
    NEXT_STATE->REGS[Rd] = (CURRENT_STATE->REGS[Rn]) - (CURRENT_STATE->REGS[Rm]);
    
    update_flags(NEXT_STATE->REGS[Rd], NEXT_STATE);
}


void update_flags(uint64_t result, CPU_State *NEXT_STATE) {
    if (result == 0) {
        NEXT_STATE->FLAG_Z = 1;
    } else {
        NEXT_STATE->FLAG_Z = 0;
    }

    if (result < 0) {
        NEXT_STATE->FLAG_N = 1;
    } else {
        NEXT_STATE->FLAG_N = 0;
    }
}