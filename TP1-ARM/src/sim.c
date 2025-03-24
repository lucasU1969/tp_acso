#include <stdio.h>
#include <assert.h>
#include <string.h>

#include<stdint.h>
#include "shell.h"

extern CPU_State CURRENT_STATE;
extern CPU_State NEXT_STATE;

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
void adds_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void adds_extended_register(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void hlt();
void ands_shifted(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void orr_shifted(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void update_flags(uint64_t result, CPU_State *NEXT_STATE);

void process_instruction(){
    /* execute one instruction here. You should use CURRENT_STATE and modify
    * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
    * access memory. 
    * */

    uint64_t PC = CURRENT_STATE.PC;
    uint32_t instruction = mem_read_32(PC);

    printf("Instruction: %x\n", instruction);

//  Masks
    int mask_11bits = 0b11111111111<<21;
    int mask_8bits = 0b11111111<<24;
    
    uint32_t adds_extended_opcode = 0b10101011001<<21;
    uint32_t adds_immediate_opcode = 0b10110001<<24;
    uint32_t hlt_opcode = 0b11010100010<<21;
    uint32_t ands_shifted_opcode = 0b11101010000<<21;
    uint32_t orr_opcode = 0b10101010000<<21;

    uint32_t mask_11bits = 0b11111111111<<21;
    uint32_t mask_8bits = 0b11111111<<24;

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

    if ((instruction & mask_11bits) == adds_extended_opcode){
        adds_extended(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_8bits) == adds_immediate_opcode) {
        adds_immediate(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == hlt_opcode){
        hlt();
    }

    if ((instruction & mask_11bits) == ands_shifted_opcode){
        ands_shifted(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == orr_opcode){
        orr_shifted(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    //Actualizo PC
    NEXT_STATE.PC += 4;
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

void adds_extended(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    //suma entre 2 operandos, 1er operando depende del registro rn. si el registro vale 31 hay que sacar el operando del stack pointer.
    //stack pointer current state current regs[31].si no es igual a 31, lo saco del registro rn current state[rn].
    //operando 2 utiliza registro[rm], option y imm3. Option va de 0 a 7, segun el numero ue tenga option, es lo que voy a tener que 
    //aplicarle a rm. Parto en 8 casos segun cada caso, aplico una mascara distinta, esa mascara se lo aplico a lo que estaba en rm.
    //Al resultado le aplico un left shift con imm3. Ahora sumo operando 1 y operando 2, actualizo las flags y guardo el res en
    //registro rd.
    printf("es un ADDS extended!!!!\n");
    uint32_t Rd = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint32_t imm3 = (instruction & 0b111<<10)>>10;
    uint32_t option = (instruction & 0b111>>13)<<13;
    uint32_t Rm = (instruction & 0b11111>>16)<<16;

    //Operando 1
    if (CURRENT_STATE -> REGS[Rn] == 31){
        Rn = CURRENT_STATE -> REGS[31];
    } else {
        Rn = CURRENT_STATE -> REGS[Rn];
    }
    //Operando 2
    Rm = CURRENT_STATE -> REGS[Rm];
    switch  (option){
        case 0b000: Rm = (uint8_t)Rm; break;
        case 0b001: Rm = (uint16_t)Rm; break;
        case 0b010: Rm = (uint32_t)Rm; break;
        case 0b011:
            Rm = (uint64_t)Rm;
            Rm = Rm << imm3;
            break;
        case 0b100: Rm = (int8_t)Rm; break;
        case 0b101: Rm = (int16_t)Rm; break;
        case 0b110: Rm = (int32_t)Rm; break;
        case 0b111: Rm = (uint64_t)Rm; break;
    }
    //Rm = Rm<<imm3;
    printf("Rd: %d\n", Rd);
    printf("Rn: %d\n", Rn);
    printf("Rm: %d\n", Rm);
    printf("Imm3: %d\n", imm3);
    printf("Option: %d\n", option);

    //Operacion y flags (por fin, que horror)
    uint64_t res = Rn + Rm;
    printf("res: %ld\n", res);
    NEXT_STATE -> REGS[Rd] = res;
    update_flags(res, NEXT_STATE);
}

void adds_immediate(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    printf("es un ADDS immediate!!!!\n");
    uint32_t Rd = instruction & 0b11111;
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint32_t imm12 = (instruction & 0b111111111111<<10)>>10;
    uint32_t shift = (instruction & 0b11<<22)>>22;
    printf("Rd: %d\n", Rd);
    printf("Rn: %d\n", Rn);
    printf("imm12: %d\n", imm12);
    printf("shift: %d\n", shift);
    if (shift == 01){
        imm12 = imm12<<12;
    }
    //Operacion y flags
    uint64_t res = CURRENT_STATE -> REGS[Rn] + imm12;
    printf("res: %ld\n", res);
    NEXT_STATE -> REGS[Rd] = res;
    update_flags(res, NEXT_STATE);
}

void hlt(){
    printf("es un HALT!!!!\n");
    extern int RUN_BIT;
    RUN_BIT = FALSE;
}

void ands_shifted(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    printf("es un ANDS shifted!!!!\n");
    uint32_t Rd = instruction & 0b11111;
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint32_t Rm = (instruction & 0b11111<<16)>>16;
    printf("Rd: %d\n", Rd);
    printf("Rn: %d\n", Rn);
    printf("Rm: %d\n", Rm);

    uint64_t res = CURRENT_STATE -> REGS[Rn] & CURRENT_STATE -> REGS[Rm];
    printf("res: %ld\n", res);
    NEXT_STATE -> REGS[Rd] = res;
    update_flags(res, NEXT_STATE);
}

void orr_shifted(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    printf("es un ORR shifted!!!!\n");
    uint32_t Rd = instruction & 0b11111;
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint32_t Rm = (instruction & 0b11111<<16)>>16;
    printf("Rd: %d\n", Rd);
    printf("Rn: %d\n", Rn);
    printf("Rm: %d\n", Rm);

    uint64_t res = CURRENT_STATE -> REGS[Rn] | CURRENT_STATE -> REGS[Rm];
    printf("res: %ld\n", res);
    NEXT_STATE -> REGS[Rd] = res;
    update_flags(res, NEXT_STATE);
}

void update_flags(uint64_t res, CPU_State *NEXT_STATE) {
    NEXT_STATE -> FLAG_N = (res >> 63) & 1;
    NEXT_STATE -> FLAG_Z = (res == 0);
}