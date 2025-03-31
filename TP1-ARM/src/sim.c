#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "shell.h"
#include "functions.h"



void process_instruction();


typedef struct instruction_t {
    uint32_t opcode;
    void (*function)(uint32_t);
} instruction_t;

void process_instruction(){
    uint64_t PC = CURRENT_STATE.PC;
    uint32_t instruction = mem_read_32(PC);

    instruction_t instructions[] = {
        {0b10110001<<24,                adds_immediate},
        {0b10101011000<<21,             adds_extended},
        {0b11110001<<24,                subs_immediate},
        {0b11101011000<<21,             subs_extended_register},
        {0b11010100010<<21,             hlt},
        {0b11101010000<<21,             ands_shifted},
        {0b11001010000<<21,             eor_shifted_register},
        {0b10101010000<<21,             orr_shifted},
        {0b000101<<26,                  b},
        {0b1101011000011111000000<<10,  br},
        {0b01010100<<24,                b_cond},
        {0b1101001101<<22,              logical_shift_immediate},
        {0b11111000010<<21,             ldur},
        {0b00111000010<<21,             ldurb},
        {0b01111000010<<21,             ldurh},
        {0b11111000000<<21,             stur},
        {0b00111000000<<21,             sturb},
        {0b01111000000<<21,             sturh},
        {0b11010010100<<21,             movz},
        {0b10011011000<<21,             mul},
        {0b10110100<<24,                cbz},
        {0b10110101<<24,                cbnz},
        {0b10010001<<24,                add_immediate},
        {0b10001011000<<21,             add_extended},
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