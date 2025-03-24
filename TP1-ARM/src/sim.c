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

void process_instruction();
void subs_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void subs_extended_register(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void cmp_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);


void update_flags(uint64_t result, CPU_State *NEXT_STATE);
int64_t option_switch(int option, int64_t operand_m, int imm3);


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

    printf("subs (extended register) opcode: %x\n", subs_extended_register_opcode);

    if ((instruction & mask_8bits) == subs_immediate_opcode){
        subs_immediate(instruction, &CURRENT_STATE, &NEXT_STATE); 
    }

    if ((instruction & mask_11bits) == subs_extended_register_opcode){
        printf("subs_extended_register\n");
        subs_extended_register(instruction, &CURRENT_STATE, &NEXT_STATE);
    }



    NEXT_STATE.PC  = PC + 4;
}


void subs_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
//  se puede hacer en términos de un add a un not immediate. 
// ya hace el cmp.

    char shift = (pars & 0b11<<22) >> 22;
    short imm12 = (pars & 0b111111111111<<10)>>10;
    if (shift) {
        imm12 = imm12 << 12;
    }
    short Rn = (pars & 0b11111<<5)>>5;
    short Rd = pars & 0b11111;

    NEXT_STATE->REGS[Rd] = (CURRENT_STATE->REGS[Rn]) - imm12;

    update_flags(NEXT_STATE->REGS[Rd], NEXT_STATE);

    if (Rd == 0b11111) {
        NEXT_STATE->REGS[Rd] = CURRENT_STATE->REGS[Rd];
    }
}


void subs_extended_register(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
// hace el cmp.
    short Rm = (pars & 0b11111<<16)>>16;
    short option = (pars & 0b111<<13)>>13; 
    short imm3 = (pars & 0b111>>10)>>10;
    short Rn = (pars & 0b11111<<5)>>5;
    short Rd = pars & 0b11111;

    int64_t operand_m = option_switch(option, CURRENT_STATE->REGS[Rm], imm3);

    NEXT_STATE->REGS[Rd] = (CURRENT_STATE->REGS[Rn]) - operand_m;
    
    update_flags(NEXT_STATE->REGS[Rd], NEXT_STATE);

    if (Rd == 0b11111) {
        NEXT_STATE->REGS[Rd] = CURRENT_STATE->REGS[Rd];
    }
}



int64_t option_switch(int option, int64_t operand_m, int imm3) {
    switch (option) {
        case 0b000: 
            operand_m = (uint32_t) (char) operand_m;
            break;
        case 0b001:
            operand_m = (uint32_t) (short) operand_m;
            break;
        case 0b010:
            operand_m = (uint64_t) (uint32_t) operand_m;
            break;
        case 0b011:
            operand_m = operand_m<<imm3;
            operand_m = (uint64_t) operand_m;
            break;
        case 0b100: 
            operand_m = (int32_t) (char) operand_m;
            break;
        case 0b101: 
            operand_m = (int32_t) (short) operand_m;
            break;
        case 0b110:
            operand_m = (int64_t) (uint32_t) operand_m;
            break;
        case 0b111: 
            operand_m = (int64_t) operand_m;
            break;
    }
    return operand_m;
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