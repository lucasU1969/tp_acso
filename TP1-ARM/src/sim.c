#include <stdio.h>
#include <assert.h>
#include <string.h>
#include<stdint.h>
#include "shell.h"


void process_instruction();


// tested:
void adds_immediate(uint32_t pars);
void adds_extended(uint32_t pars);
void subs_immediate(uint32_t pars);
void subs_extended_register(uint32_t pars);
void hlt(uint32_t instruction);
// cmp immediate
// cmp extended_register
void ands_shifted(uint32_t pars);
void eor_shifted_register(uint32_t instruction);
void orr_shifted(uint32_t instruction);
void b_cond(uint32_t instruction);
void logical_shift_immediate(uint32_t instruction);
void stur(uint32_t instruction);
void sturb(uint32_t instruction);
void sturh(uint32_t instruction);
void movz(uint32_t instruction);
void mul(uint32_t instruction);


// to test:
void cbz(uint32_t instruction);   
void cbnz(uint32_t instruction); 

void ldur(uint32_t instruction);
void ldurb(uint32_t instruction);
void ldurh(uint32_t instruction);
void b(uint32_t instruction);
void br(uint32_t instruction);
void add_immediate(uint32_t instruction);
void add_extended(uint32_t instruction);


void update_flags(uint64_t result);
int64_t sign_extend(int64_t value, int64_t idx);

// typedef struct instruction_t {
//     uint32_t opcode;
//     void (*function)(uint32_t);
// } instruction_t;

void process_instruction(){
    /* execute one instruction here. You should use CURRENT_STATE and modify
    * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
    * access memory. 
    * */

    uint64_t PC = CURRENT_STATE.PC;
    uint32_t instruction = mem_read_32(PC);

    // printf("Instruction: %x\n", instruction);

//  Masks
    uint32_t mask_22bits = 0b1111111111111111111111<<10;
    uint32_t mask_11bits = 0b11111111111<<21;
    uint32_t mask_8bits = 0b11111111<<24;
    uint32_t mask_6bits = 0b111111<<26;
    uint32_t mask_9bits = 0b111111111<<23;

//  Opcodes
    uint32_t adds_extended_opcode = 0b10101011000<<21;
    uint32_t adds_immediate_opcode = 0b10110001<<24;
    uint32_t subs_immediate_opcode = 0b11110001<<24;
    uint32_t subs_extended_register_opcode = 0b11101011000<<21;
    uint32_t hlt_opcode = 0b11010100010<<21;
    uint32_t ands_shifted_opcode = 0b11101010000<<21;
    uint32_t eor_shifted_register_opcode = 0b11001010000<<21;
    uint32_t orr_opcode = 0b10101010000<<21;
    uint32_t b_opcode = 0b000101<<26;
    uint32_t br_opcode = 0b1101011000011111000000<<10;
    uint32_t b_cond_opcode = 0b01010100<<24;
    uint32_t logical_shift_immediate_opcode = 0b110100110<<23;
    uint32_t stur_opcode = 0b11111000000<<21;
    uint32_t sturb_opcode = 0b00111000000<<21;
    uint32_t sturh_opcode = 0b01111000000<<21;
    uint32_t ldur_opcode = 0b11111000010<<21;
    uint32_t movz_opcode = 0b11010010100<<21;
    uint32_t mul_opcode =  0b10011011000<<21;
    uint32_t cbz_opcode = 0b10110100<<24;
    uint32_t cbnz_opcode = 0b10110101<<24;
    uint32_t add_immediate_opcode = 0b10010001<<24;
    uint32_t add_extended_opcode = 0b10001011001<<21;
    uint32_t ldurb_opcode = 0b00111000010<<21;
    uint32_t ldurh_opcode = 0b01111000010<<21;

    char program_counter_increase = 1;


    if ((instruction & mask_8bits) == subs_immediate_opcode){
        subs_immediate(instruction); 
    }

    if ((instruction & mask_11bits) == subs_extended_register_opcode){
        subs_extended_register(instruction);
    }

    if ((instruction & mask_11bits) == adds_extended_opcode){
        adds_extended(instruction);
    }

    if ((instruction & mask_8bits) == adds_immediate_opcode) {
        adds_immediate(instruction);
    }

    if ((instruction & mask_11bits) == hlt_opcode){
        hlt(instruction);
    }

    if ((instruction & mask_11bits) == ands_shifted_opcode){
        ands_shifted(instruction);
    }

    if ((instruction & mask_11bits) == orr_opcode){
        orr_shifted(instruction);
    }

    if ((instruction & mask_6bits) == b_opcode){
        b(instruction);
        program_counter_increase = 0;
    }

    if ((instruction & mask_11bits) == eor_shifted_register_opcode){
        eor_shifted_register(instruction);
    }

    if ((instruction & mask_22bits)== br_opcode){
        br(instruction);
        program_counter_increase = 0;
    }

    if ((instruction & mask_8bits) == b_cond_opcode){
        b_cond(instruction);
        program_counter_increase = 0;
    }

    if ((instruction & mask_9bits) == logical_shift_immediate_opcode){
        logical_shift_immediate(instruction);
    }

    if ((instruction & mask_11bits) == stur_opcode){
        stur(instruction);
    }

    if ((instruction & mask_11bits) == movz_opcode){
        movz(instruction);
    }

    if ((instruction & mask_11bits) == mul_opcode) {
        mul(instruction);
    }

    if ((instruction & mask_11bits) == sturb_opcode){
        sturb(instruction);
    }
    
    if ((instruction & mask_11bits) == sturh_opcode){
        sturh(instruction);
    }
    
    if ((instruction & mask_11bits) == ldur_opcode){
        ldur(instruction);
    }

    if ((instruction & mask_8bits) == cbz_opcode){
        cbz(instruction);
        program_counter_increase = 0;
    }
    
    if ((instruction & mask_8bits) == cbnz_opcode){
        cbnz(instruction);
        program_counter_increase = 0;
    }

    if ((instruction & mask_8bits) == add_immediate_opcode){
        add_immediate(instruction);
    }

    if ((instruction & mask_11bits) == add_extended_opcode){
        add_extended(instruction);
    }

    if ((instruction & mask_11bits) == ldurb_opcode){
        ldurb(instruction);
    }

    if ((instruction & mask_11bits) == ldurh_opcode){
        ldurh(instruction);
    }

    if (program_counter_increase) {
        NEXT_STATE.PC += 4;
    }

    NEXT_STATE.REGS[31] = CURRENT_STATE.REGS[31];
}

void subs_immediate(uint32_t pars) {
    char shift = (pars & 0b11<<22) >> 22;
    short imm12 = (pars & 0b111111111111<<10)>>10;
    if (shift) {
        imm12 = imm12 << 12;
    }
    short Rn = (pars & 0b11111<<5)>>5;
    short Rd = pars & 0b11111;

    NEXT_STATE.REGS[Rd] = (CURRENT_STATE.REGS[Rn]) - imm12;

    update_flags(NEXT_STATE.REGS[Rd]);
}

void subs_extended_register(uint32_t pars) {
    short Rm = (pars & 0b11111<<16)>>16;
    short option = (pars & 0b111<<13)>>13; 
    short imm3 = (pars & 0b111>>10)>>10;
    short Rn = (pars & 0b11111<<5)>>5;
    short Rd = pars & 0b11111;

    NEXT_STATE.REGS[Rd] = (CURRENT_STATE.REGS[Rn]) - (CURRENT_STATE.REGS[Rm]);
    
    update_flags(NEXT_STATE.REGS[Rd]);
}

void adds_extended(uint32_t instruction){
    uint32_t Rd = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    uint64_t imm3 = (instruction & 0b111<<10)>>10;
    uint32_t option = (instruction & 0b111>>13)<<13;
    uint32_t Rm = (instruction & 0b11111>>16)<<16;
    uint64_t res = Rn + Rm;
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
    uint32_t imm26 = (instruction & 0b11111111111111111111111111)<<2;
    NEXT_STATE.PC += (int64_t) imm26;
}

void br(uint32_t instruction){
    uint32_t Rm = (instruction & 0b11111<<5)>>5;
    uint64_t target = CURRENT_STATE.REGS[Rm];
    NEXT_STATE.PC = target;
}

void b_cond(uint32_t instruction){
    int64_t imm19 = sign_extend(instruction>>5, 18);

    int64_t offset = imm19 << 2;
    int8_t cond = (instruction & 0b1111);

    switch (cond) {
        case 0b0000: // EQ 
            if (CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC += offset;
            }
            break;
        case 0b0001: // NE
            if (!(CURRENT_STATE.FLAG_Z)) {
                NEXT_STATE.PC += offset;
            }
            break;
        case 0b1100: // GT
            if (!(CURRENT_STATE.FLAG_N) && !(CURRENT_STATE.FLAG_Z)) {
                NEXT_STATE.PC += offset;
            }
            break;
        case 0b1011: // LT
            if ((CURRENT_STATE.FLAG_N) && !(CURRENT_STATE.FLAG_Z)) {
                NEXT_STATE.PC += offset;
            }
            break;
        case 0b1010: // GE
            if (!(CURRENT_STATE.FLAG_N) || (CURRENT_STATE.FLAG_Z)) {
                NEXT_STATE.PC += offset;
            }
            break;
        case 0b1101: // LE
            if ((CURRENT_STATE.FLAG_N) || (CURRENT_STATE.FLAG_Z)) {
                NEXT_STATE.PC += offset;
            }
            break;
        default:
            NEXT_STATE.PC += 4;
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
    int16_t imm9;
    int64_t offset = sign_extend(instruction>>12, 8);
    int64_t address = CURRENT_STATE.REGS[Rn] + offset;
    int32_t lower_data_bits = mem_read_32(address) & 0xFF;
    NEXT_STATE.REGS[Rt] = lower_data_bits;
}

void ldurh(uint32_t instruction){
    uint32_t Rt = (instruction & 0b11111);
    uint32_t Rn = (instruction & 0b11111<<5)>>5;
    int16_t imm9;
    int64_t offset = sign_extend(instruction>>12, 8);
    int64_t address = CURRENT_STATE.REGS[Rn] + offset;
    int32_t data = mem_read_32(address);
    int32_t lower_data_bits = mem_read_32(address) & 0xFFFF<<16;
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
    int64_t imm19;
    int64_t offset = sign_extend(instruction>>5, 18) << 2;
    
    if (CURRENT_STATE.REGS[Rt] == 0) {
        NEXT_STATE.PC += offset;
    }
    NEXT_STATE.PC += 4;
}

void cbnz(uint32_t instruction) {
    uint8_t Rt = instruction & 0b11111;
    int64_t imm19;
    int64_t offset = sign_extend(instruction>>5, 18) << 2;
    
    if (CURRENT_STATE.REGS[Rt] != 0) {
        NEXT_STATE.PC += offset;
    }
    NEXT_STATE.PC += 4;
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
    uint64_t imm3 = (instruction & 0b111<<10)>>10;
    uint32_t option = (instruction & 0b111>>13)<<13;
    uint32_t Rm = (instruction & 0b11111>>16)<<16;
    uint64_t res = Rn + Rm;
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