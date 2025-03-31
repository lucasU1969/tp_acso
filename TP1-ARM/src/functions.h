#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "shell.h"
#include <stdlib.h>

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