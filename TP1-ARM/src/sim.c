#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "shell.h"
#include <stdlib.h>


void process_instruction();


// MANEJO DE POSICIONES DE .DATA INVÁLIDAS
// MANEJO DE POSICIONES DE .TEXT INVÁLIDAS

void adds_immediate(uint32_t instruction);
void adds_extended(uint32_t instruction);
void subs_immediate(uint32_t instruction);
void subs_extended_register(uint32_t instruction);
void hlt(uint32_t instruction);
void ands_shifted(uint32_t instruction);
void eor_shifted_register(uint32_t instruction);
void orr_shifted(uint32_t instruction);
void b_cond(uint32_t instruction);
void logical_shift_immediate(uint32_t instruction);
void stur(uint32_t instruction);
void sturb(uint32_t instruction);
void sturh(uint32_t instruction);
void movz(uint32_t instruction);
void mul(uint32_t instruction);
void b(uint32_t instruction);
void br(uint32_t instruction);
void cbz(uint32_t instruction);   
void cbnz(uint32_t instruction); 
void ldurb(uint32_t instruction);
void ldur(uint32_t instruction);
void ldurh(uint32_t instruction);
void add_immediate(uint32_t instruction);
void add_extended(uint32_t instruction);


void update_flags(uint64_t result);
int64_t sign_extend(int64_t value, int64_t idx);
void jump_if(int64_t offset, int8_t bool);

typedef struct instruction_t {
    uint32_t opcode;
    void (*function)(uint32_t);
} instruction_t;

void process_instruction(){
    uint64_t PC = CURRENT_STATE.PC;
    uint32_t instruction = mem_read_32(PC);

    instruction_t instructions[] = {
        {0b10110001<<24, adds_immediate},
        {0b10101011000<<21, adds_extended},
        {0b11110001<<24, subs_immediate},
        {0b11101011000<<21, subs_extended_register},
        {0b11010100010<<21, hlt},
        {0b11101010000<<21, ands_shifted},
        {0b11001010000<<21, eor_shifted_register},
        {0b10101010000<<21, orr_shifted},
        {0b000101<<26, b},
        {0b1101011000011111000000<<10, br},
        {0b01010100<<24, b_cond},
        {0b1101001101<<22, logical_shift_immediate},
        {0b11111000010<<21, ldur},
        {0b00111000010<<21, ldurb},
        {0b01111000010<<21, ldurh},
        {0b11111000000<<21, stur},
        {0b00111000000<<21, sturb},
        {0b01111000000<<21, sturh},
        {0b11010010100<<21, movz},
        {0b10011011000<<21, mul},
        {0b10110100<<24, cbz},
        {0b10110101<<24, cbnz},
        {0b10010001<<24, add_immediate},
        {0b10001011000<<21, add_extended},
    };

    uint32_t masks[] = {
        0b1111111111111111111111<<10,
        0b11111111111<<21,
        0b1111111111<<22,
        0b11111111<<24,
        0b111111<<26,
    };

    int num_instructions = sizeof(instructions) / sizeof(instructions[0]);

    for (int mask = 0; mask < 5; mask++) {
        for (int i = 0; i < num_instructions; i++) {    
            if ((instruction & masks[mask]) == instructions[i].opcode) {
                instructions[i].function(instruction);
                NEXT_STATE.PC += 4;
                NEXT_STATE.REGS[31] = CURRENT_STATE.REGS[31];
                return;
            }
        }
    }  
    printf("Error: Instruction not found\n");
    exit(1);  
}

void subs_immediate(uint32_t instruction) {
    char shift = (instruction & 0b11<<22) >> 22;
    short imm12 = (instruction & 0b111111111111<<10)>>10;
    if (shift) {
        imm12 = imm12 << 12;
    }
    short Rn = (instruction & 0b11111<<5)>>5;
    short Rd = instruction & 0b11111;

    NEXT_STATE.REGS[Rd] = (CURRENT_STATE.REGS[Rn]) - imm12;

    update_flags(NEXT_STATE.REGS[Rd]);
}

void subs_extended_register(uint32_t instruction) {
    short Rm = (instruction & 0b11111<<16)>>16;
    short Rn = (instruction & 0b11111<<5)>>5;
    short Rd = instruction & 0b11111;

    NEXT_STATE.REGS[Rd] = (CURRENT_STATE.REGS[Rn]) - (CURRENT_STATE.REGS[Rm]);
    
    update_flags(NEXT_STATE.REGS[Rd]);
}

void adds_extended(uint32_t instruction){
    uint32_t Rd = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint32_t Rm = (instruction & 0b11111<<16)>>16;
    uint64_t res = CURRENT_STATE.REGS[Rn] + CURRENT_STATE.REGS[Rm];
    NEXT_STATE.REGS[Rd] = res;
    update_flags(res);
}

void adds_immediate(uint32_t instruction){
    uint32_t Rd = instruction & 0b11111;
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint64_t imm12 = (instruction & 0b111111111111<<10)>>10;
    uint32_t shift = (instruction & 0b11<<22)>>22;
    if (shift == 01){
        imm12 = imm12<<12;
    }
    uint64_t res = CURRENT_STATE.REGS[Rn] + imm12;
    NEXT_STATE.REGS[Rd] = res;
    update_flags(res);
}

void hlt(uint32_t instruction){
    RUN_BIT = FALSE;
}

void ands_shifted(uint32_t instruction){
    uint32_t Rd = instruction & 0b11111;
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint32_t Rm = (instruction & 0b11111<<16)>>16;
    uint64_t res = CURRENT_STATE.REGS[Rn] & CURRENT_STATE.REGS[Rm];
    NEXT_STATE.REGS[Rd] = res;
    update_flags(res);
}

void orr_shifted(uint32_t instruction){
    uint32_t Rd = instruction & 0b11111;
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint32_t Rm = (instruction & 0b11111<<16)>>16;
    uint64_t res = CURRENT_STATE.REGS[Rn] | CURRENT_STATE.REGS[Rm];
    NEXT_STATE.REGS[Rd] = res;
    update_flags(res);
}

void eor_shifted_register(uint32_t instruction) {
    short Rd = instruction & 0b11111;
    short Rn = (instruction & 0b11111<<5)>>5;
    short Rm = (instruction & 0b11111<<16)>>16;
    short imm6 = (instruction & 0b111111<<10)>>10;
    
    NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] ^ CURRENT_STATE.REGS[Rm];
}

void b(uint32_t instruction){
    int64_t target = sign_extend(instruction, 25)<<2;
    NEXT_STATE.PC += target-4;
}

void br(uint32_t instruction){
    uint32_t Rm = (instruction & 0b11111<<5)>>5;
    uint64_t target = CURRENT_STATE.REGS[Rm];
    NEXT_STATE.PC = target-4;
}

void b_cond(uint32_t instruction){
    int64_t imm19 = sign_extend(instruction>>5, 18);

    int64_t offset = imm19 << 2;
    int8_t cond = (instruction & 0b1111);

    switch (cond) {
        case 0b0000: // EQ 
            jump_if(offset, CURRENT_STATE.FLAG_Z);
            break;
        case 0b0001: // NE
            jump_if(offset, !CURRENT_STATE.FLAG_Z);
            break;
        case 0b1100: // GT
            jump_if(offset, !(CURRENT_STATE.FLAG_N) && !(CURRENT_STATE.FLAG_Z));
            break;
        case 0b1011: // LT
            jump_if(offset, (CURRENT_STATE.FLAG_N) && !(CURRENT_STATE.FLAG_Z));
            break;
        case 0b1010: // GE
            jump_if(offset, !(CURRENT_STATE.FLAG_N) || (CURRENT_STATE.FLAG_Z));
            break;
        case 0b1101: // LE
            jump_if(offset, (CURRENT_STATE.FLAG_N) || (CURRENT_STATE.FLAG_Z));
            break;
    }
}

void logical_shift_immediate(uint32_t instruction){
    uint8_t Rd = instruction & 0b11111;
    uint8_t Rn = (instruction & 0b11111<<5)>>5;
    uint8_t imms = instruction>>10 & 0b111111;
    uint8_t immr = instruction>>16 & 0b111111;

    if (imms == 0b111111) {
        NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] >> immr;
    } else {
        NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] << 64 - immr;
    }
}


void stur(uint32_t instruction){
    int64_t offset = sign_extend(instruction>>12, 8);
    uint8_t Rn = instruction>>5 & 0b11111;
    uint8_t Rt = instruction & 0b11111;
    int64_t address = CURRENT_STATE.REGS[Rn] + offset;
    int64_t data = CURRENT_STATE.REGS[Rt];
    int32_t higher_data_bits = data >> 32;
    int32_t lower_data_bits = data & 0xFFFFFFFF;

    mem_write_32(address, lower_data_bits);
    mem_write_32(address + 0x4, higher_data_bits);
}

void sturb(uint32_t instruction){
    int64_t offset = sign_extend(instruction>>12, 8);
    uint8_t Rn = instruction>>5 & 0b11111;
    uint8_t Rt = instruction & 0b11111;
    int64_t address = CURRENT_STATE.REGS[Rn] + offset;
    int64_t data = CURRENT_STATE.REGS[Rt];
    int32_t lower_data_bits = data & 0xFF;

    uint32_t mem = mem_read_32(address);
    mem = mem & 0xFFFFFF00;
    mem = mem | lower_data_bits;
    mem_write_32(address, mem);
}

void sturh(uint32_t instruction) {
    int64_t offset = sign_extend(instruction>>12, 8);
    uint8_t Rn = instruction>>5 & 0b11111;
    uint8_t Rt = instruction & 0b11111;
    int64_t address = CURRENT_STATE.REGS[Rn] + offset;
    int64_t data = CURRENT_STATE.REGS[Rt];
    int32_t lower_data_bits = data & 0xFFFF;

    uint32_t mem = mem_read_32(address);

    mem = mem & 0xFFFF0000;
    mem = mem | lower_data_bits;

    mem_write_32(address, mem);
}

void ldur(uint32_t instruction){
    int64_t offset = sign_extend(instruction>>12, 8);
    uint32_t Rt = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    int64_t address = CURRENT_STATE.REGS[Rn] + offset;
    int64_t lower_data_bits = mem_read_32(address);
    int64_t higher_data_bits = mem_read_32(address + 0x4);
    int64_t data = (higher_data_bits << 32) | lower_data_bits;
    NEXT_STATE.REGS[Rt] = data;
}


void ldurb(uint32_t instruction){
    uint32_t Rt = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    int64_t offset = sign_extend(instruction>>12, 8);
    int64_t address = CURRENT_STATE.REGS[Rn] + offset;
    int32_t lower_data_bits = mem_read_32(address) & 0xFF;
    NEXT_STATE.REGS[Rt] = lower_data_bits;
}

void ldurh(uint32_t instruction){
    uint32_t Rt = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    int64_t offset = sign_extend(instruction>>12, 8);
    int64_t address = CURRENT_STATE.REGS[Rn] + offset;
    int32_t lower_data_bits = mem_read_32(address) & 0xFFFF;
    NEXT_STATE.REGS[Rt] = lower_data_bits;
}

void movz(uint32_t instruction){
    uint8_t Rd = instruction & 0b11111;
    uint16_t imm16 = instruction >> 5 & 0xFFFF;
    
    NEXT_STATE.REGS[Rd] = imm16;
}


void mul(uint32_t instruction) {
    uint8_t Rm = instruction>>16 & 0b11111;
    uint8_t Rn = instruction>>5 & 0b11111;
    uint8_t Rd = instruction & 0b11111;
    
    NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] * CURRENT_STATE.REGS[Rm];
    update_flags(NEXT_STATE.REGS[Rd]);
}


void cbz(uint32_t instruction){
    uint8_t Rt = instruction & 0b11111;
    int64_t offset = sign_extend(instruction>>5, 18) << 2;
    jump_if(offset, CURRENT_STATE.REGS[Rt] == 0);
}

void cbnz(uint32_t instruction) {
    uint8_t Rt = instruction & 0b11111;
    int64_t offset = sign_extend(instruction>>5, 18) << 2;
    jump_if(offset, CURRENT_STATE.REGS[Rt] != 0);
}

void add_immediate(uint32_t instruction){
    uint32_t Rd = instruction & 0b11111;
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint64_t imm12 = (instruction & 0b111111111111<<10)>>10;
    uint32_t shift = (instruction & 0b11<<22)>>22;
    if (shift == 01){
        imm12 = imm12<<12;
    }
    uint64_t res = CURRENT_STATE.REGS[Rn] + imm12;
    NEXT_STATE.REGS[Rd] = res;
}

void add_extended(uint32_t instruction){
    uint32_t Rd = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint32_t Rm = (instruction & 0b11111<<16)>>16;
    uint64_t res = CURRENT_STATE.REGS[Rn] + CURRENT_STATE.REGS[Rm];
    NEXT_STATE.REGS[Rd] = res;
}



void update_flags(uint64_t res) {
    NEXT_STATE.FLAG_N = (res >> 63) & 1;
    NEXT_STATE.FLAG_Z = (res == 0);
}

int64_t sign_extend(int64_t value, int64_t idx) {
    int64_t res;
    if (value >> idx & 0b1) {
        res = value | (0xFFFFFFFFFFFFFFFF << idx);
    } else {
        res = value & ~(0xFFFFFFFFFFFFFFFF << (idx+1));
    }
    return res;
}

void jump_if(int64_t offset, int8_t bool) {
    if (bool) {
        NEXT_STATE.PC += offset-4;
    }
}