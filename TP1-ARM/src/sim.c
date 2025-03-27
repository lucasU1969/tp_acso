#include <stdio.h>
#include <assert.h>
#include <string.h>

#include<stdint.h>
#include "shell.h"

extern CPU_State CURRENT_STATE;
extern CPU_State NEXT_STATE;
extern int RUN_BIT;

/*
    PREGUNTAR: 
        1. LSL (immediate): por qué los valores de los immediates están mal? hay que hacer una operación entre los dos immediates
        2. EOR (shifted register): actualiza flags? no
        3. ADDS (extended register), SUBS (extended register): option switch?
        4. En qué caso se usa el stack? nunca
        5. Casos en los que Rn = 31? 
*/


/*
se puede modelar como punteros a función. 
no importa el orden en el que se empieza a comparar. 
pasar parámetros a main. 
*/
void process_instruction();


// tested: 
void adds_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void subs_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void subs_extended_register(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
// cmp extended_register
// cmp immediate
void eor_shifted_register(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void b_cond(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void logical_shift_immediate(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void stur(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void sturb(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void sturh(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void movz(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void mul(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);


// to test:
void cbz(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);   
void cbnz(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE); 


void hlt(int *RUN_BIT);
void ands_shifted(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void orr_shifted(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void b(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);
void br(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);

// sus: 
void adds_extended(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE);


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
    uint32_t mask_22bits = 0b1111111111111111111111<<10;
    uint32_t mask_11bits = 0b11111111111<<21;
    uint32_t mask_8bits = 0b11111111<<24;
    uint32_t mask_6bits = 0b111111<<26;
    uint32_t mask_9bits = 0b111111111<<23;

//  Opcodes
    uint32_t adds_extended_opcode = 0b10101011001<<21;
    uint32_t adds_immediate_opcode = 0b10110001<<24;
    uint32_t subs_immediate_opcode = 0b11110001<<24;
    uint32_t subs_extended_register_opcode = 0b11101011000<<21;
    uint32_t hlt_opcode = 0b11010100010<<21;
    uint32_t ands_shifted_opcode = 0b11101010000<<21;
    uint32_t eor_shifted_register_opcode = 0b11001010000<<21;
    uint32_t orr_opcode = 0b10101010000<<21;
    uint32_t b_opcode = 0b000101<<26;
    uint32_t br_opcode = 0b11010110000111110000<<10;
    uint32_t b_cond_opcode = 0b01010100<<24;
    uint32_t logical_shift_immediate_opcode = 0b110100110<<23;
    uint32_t stur_opcode = 0b11111000000<<21;
    uint32_t sturb_opcode = 0b00111000000<<21;
    uint32_t sturh_opcode = 0b01111000000<<21;
    uint32_t movz_opcode = 0b11010010100<<21;
    uint32_t mul_opcode =  0b10011011000<<21;
    uint32_t cbz_opcode = 0b10110100<<24;
    uint32_t cbnz_opcode = 0b10110101<<24;

    char program_counter_increase = 1;

    if ((instruction & mask_8bits) == subs_immediate_opcode){
        subs_immediate(instruction, &CURRENT_STATE, &NEXT_STATE); 
    }

    if ((instruction & mask_11bits) == subs_extended_register_opcode){
        subs_extended_register(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == adds_extended_opcode){
        adds_extended(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_8bits) == adds_immediate_opcode) {
        adds_immediate(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == hlt_opcode){
        hlt(&RUN_BIT);
    }

    if ((instruction & mask_11bits) == ands_shifted_opcode){
        ands_shifted(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == orr_opcode){
        orr_shifted(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_6bits) == b_opcode){
        b(instruction, &CURRENT_STATE, &NEXT_STATE);
        program_counter_increase = 0;
    }

    if ((instruction & mask_11bits) == eor_shifted_register_opcode){
        eor_shifted_register(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_22bits)== br_opcode){
        br(instruction, &CURRENT_STATE, &NEXT_STATE);
        program_counter_increase = 0;
    }

    if ((instruction & mask_8bits) == b_cond_opcode){
        b_cond(instruction, &CURRENT_STATE, &NEXT_STATE);
        program_counter_increase = 0;
    }

    if ((instruction & mask_9bits) == logical_shift_immediate_opcode){
        logical_shift_immediate(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == stur_opcode){
        stur(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == movz_opcode){
        movz(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == mul_opcode) {
        mul(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_8bits) == cbz_opcode){
        cbz(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == sturb_opcode){
        sturb(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_11bits) == sturh_opcode){
        sturh(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    if ((instruction & mask_8bits) == cbnz_opcode){
        cbnz(instruction, &CURRENT_STATE, &NEXT_STATE);
    }

    //Actualizo PC
    if (program_counter_increase) {
        NEXT_STATE.PC += 4;
    }

//  El registro 31 queda en 0
    NEXT_STATE.REGS[31] = CURRENT_STATE.REGS[31];
}

void subs_immediate(uint32_t pars, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
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

void adds_extended(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    uint32_t Rd = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint64_t imm3 = (instruction & 0b111<<10)>>10;
    uint32_t option = (instruction & 0b111>>13)<<13;
    uint32_t Rm = (instruction & 0b11111>>16)<<16;
    uint64_t res = Rn + Rm;
    NEXT_STATE -> REGS[Rd] = res;
    update_flags(res, NEXT_STATE);
}

void adds_immediate(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    uint32_t Rd = instruction & 0b11111;
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint64_t imm12 = (instruction & 0b111111111111<<10)>>10;
    uint32_t shift = (instruction & 0b11<<22)>>22;
    if (shift == 01){
        imm12 = imm12<<12;
    }
    uint64_t res = CURRENT_STATE -> REGS[Rn] + imm12;
    NEXT_STATE -> REGS[Rd] = res;
    update_flags(res, NEXT_STATE);
}

void hlt(int *RUN_BIT){
    printf("es un HALT!!!!\n");
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

void eor_shifted_register(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
    short Rd = instruction & 0b11111;
    short Rn = (instruction & 0b11111<<5)>>5;
    short Rm = (instruction & 0b11111<<16)>>16;
    short imm6 = (instruction & 0b111111<<10)>>10;
    
    NEXT_STATE -> REGS[Rd] = CURRENT_STATE -> REGS[Rn] ^ CURRENT_STATE -> REGS[Rm];
}

void b(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    uint32_t imm26 = (instruction & 0b11111111111111111111111111)<<2;
    NEXT_STATE -> PC += (int64_t) imm26;
}

void br(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    uint32_t Rm = (instruction & 0b11111<<5)>>5;
    uint64_t target = CURRENT_STATE -> REGS[Rm];
    NEXT_STATE -> PC = target;
}

void b_cond(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    int64_t imm19;
    if (instruction & 0b1<<23) {
        int32_t masked_instruction = instruction | 0b11111111<<24;
        imm19 = (masked_instruction & 0b111111111111111111111111111<<5)>>5;
    } else {
        imm19 = (instruction & 0b1111111111111111111<<5)>>5;
    }
    int64_t offset = imm19 << 2;
    short cond = (instruction & 0b1111);

    switch (cond) {
        case 0b0000: // EQ 
            if (CURRENT_STATE -> FLAG_Z) {
                NEXT_STATE -> PC += offset;
            }
            break;
        case 0b0001: // NE
            if (!(CURRENT_STATE -> FLAG_Z)) {
                NEXT_STATE -> PC += offset;
            }
            break;
        case 0b1100: // GT
            if (!(CURRENT_STATE -> FLAG_N) && !(CURRENT_STATE -> FLAG_Z)) {
                NEXT_STATE -> PC += offset;
            }
            break;
        case 0b1011: // LT
            if ((CURRENT_STATE -> FLAG_N) && !(CURRENT_STATE -> FLAG_Z)) {
                NEXT_STATE -> PC += offset;
            }
            break;
        case 0b1010: // GE
            if (!(CURRENT_STATE -> FLAG_N) || (CURRENT_STATE -> FLAG_Z)) {
                NEXT_STATE -> PC += offset;
            }
            break;
        case 0b1101: // LE
            if ((CURRENT_STATE -> FLAG_N) || (CURRENT_STATE -> FLAG_Z)) {
                NEXT_STATE -> PC += offset;
            }
            break;
    }
    NEXT_STATE -> PC += 4;
}

void logical_shift_immediate(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    uint8_t Rd = instruction & 0b11111;
    uint8_t Rn = (instruction & 0b11111<<5)>>5;
    uint8_t imms = instruction>>10 & 0b111111;
    uint8_t immr = instruction>>16 & 0b111111;

    if (imms == 0b111111) {
        NEXT_STATE -> REGS[Rd] = CURRENT_STATE -> REGS[Rn] >> immr;
    } else {
        NEXT_STATE -> REGS[Rd] = CURRENT_STATE -> REGS[Rn] << 64 - immr;
    }
}


void stur(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    int16_t imm9;
    if (instruction >> 20 & 0b1) {
        int16_t imm9 = (instruction>>12) | 0b1111111<<10;
    } else {
        imm9 = (instruction>>12) & 0b111111111;
    }
    uint8_t Rn = instruction>>5 & 0b11111;
    uint8_t Rt = instruction & 0b11111;
    int64_t offset = (int64_t) imm9;
    int64_t address = CURRENT_STATE -> REGS[Rn] + offset;
    int64_t data = CURRENT_STATE -> REGS[Rt];
    int32_t higher_data_bits = data >> 32;
    int32_t lower_data_bits = data & 0xFFFFFFFF;
    mem_write_32(address, lower_data_bits);
    mem_write_32(address + 4, higher_data_bits);
}

void sturb(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    int16_t imm9;
    if (instruction >> 20 & 0b1) {
        imm9 = (instruction>>12) | 0b1111111<<10;
    } else {
        imm9 = instruction>>12 & 0b111111111;
    }
    
    uint8_t Rn = instruction>>5 & 0b11111;
    uint8_t Rt = instruction & 0b11111;
    int64_t offset = (int64_t) imm9;
    int64_t address = CURRENT_STATE -> REGS[Rn] + offset;
    int64_t data = CURRENT_STATE -> REGS[Rt];
    int32_t lower_data_bits = data & 0xFF;

    mem_write_32(address, lower_data_bits);
}

void sturh(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
    int16_t imm9;
    if (instruction >> 20 & 0b1) {
        imm9 = (instruction>>12) | 0b1111111<<10;
    } else {
        imm9 = instruction>>12 & 0b111111111;
    }

    uint8_t Rn = instruction>>5 & 0b11111;
    uint8_t Rt = instruction & 0b11111;
    int64_t offset = (int64_t) imm9;
    int64_t address = CURRENT_STATE -> REGS[Rn] + offset;
    int64_t data = CURRENT_STATE -> REGS[Rt];
    int32_t lower_data_bits = data & 0xFFFF;

    mem_write_32(address, lower_data_bits);
}

void movz(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    uint8_t Rd = instruction & 0b11111;
    uint16_t imm16 = instruction >> 5 & 0xFFFF;

    NEXT_STATE -> REGS[Rd] = imm16;
}


void mul(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
    uint8_t Rm = instruction>>16 & 0b11111;
    uint8_t Rn = instruction>>5 & 0b11111;
    uint8_t Rd = instruction & 0b11111;

    NEXT_STATE -> REGS[Rd] = CURRENT_STATE -> REGS[Rn] * CURRENT_STATE -> REGS[Rm];
    update_flags(NEXT_STATE -> REGS[Rd], NEXT_STATE);
}


void cbz(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE){
    uint8_t Rt = instruction & 0b11111;
    int64_t imm19;
    if (instruction>>23 & 0b1){
        imm19 = (instruction>>5 & 0b1111111111111111111) | 0b1111111111111111111<<5;
    } else {
        imm19 = instruction>>5 & 0b1111111111111111111;
    }
    int64_t offset = imm19 << 2;

    if (CURRENT_STATE -> REGS[Rt] == 0) {
        NEXT_STATE -> PC += offset;
    }
    NEXT_STATE -> PC += 4;
}

void cbnz(uint32_t instruction, CPU_State *CURRENT_STATE, CPU_State *NEXT_STATE) {
    uint8_t Rt = instruction & 0b11111;
    int64_t imm19;
    if (instruction>>23 & 0b1){
        imm19 = (instruction>>5 & 0b1111111111111111111) | 0b1111111111111111111<<5;
    } else {
        imm19 = instruction>>5 & 0b1111111111111111111;
    }
    int64_t offset = imm19 << 2;

    if (CURRENT_STATE -> REGS[Rt] != 0) {
        NEXT_STATE -> PC += offset;
    }
    NEXT_STATE -> PC += 4;
}


void update_flags(uint64_t res, CPU_State *NEXT_STATE) {
    NEXT_STATE -> FLAG_N = (res >> 63) & 1;
    NEXT_STATE -> FLAG_Z = (res == 0);
}