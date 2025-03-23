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


void process_instruction(){
    /* execute one instruction here. You should use CURRENT_STATE and modify
    * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
    * access memory. 
    * */
    uint64_t PC = CURRENT_STATE.PC;
    int64_t* registers = CURRENT_STATE.REGS;
    uint32_t instruction = mem_read_32(PC);
    // int flag_n = CURRENT_STATE.FLAG_N;
    // int flag_z = CURRENT_STATE.FLAG_Z;

    //Para chequear lo que estamos leyendo
    printf("Instruction: %x\n", instruction);

    uint32_t adds_extended_opcode = 0b10101011001<<24;
    uint32_t adds_immediate_opcode = 0b10110001<<24;
    uint32_t mask_11bits = 0b11111111111<<24;
    uint32_t mask_8bits = 0b11111111<<24;

    if ((instruction & mask_11bits) == adds_extended_opcode){
    //suma entre 2 operandos, 1er operando depende del registro rn. si el registro vale 31 hay que sacar el operando del stack pointer.
    //stack pointer current state current regs[31].si no es igual a 31, lo saco del registro rn current state[rn].
    //operando 2 utiliza registro[rm], option y imm3. Option va de 0 a 7, segun el numero ue tenga option, es lo que voy a tener que 
    //aplicarle a rm. Parto en 8 casos segun cada caso, aplico una mascara distinta, esa mascara se lo aplico a lo que estaba en rm.
    //Al resultado le aplico un left shift con imm3. Ahora sumo operando 1 y operando 2, actualizo las flags y guardo el res en
    //registro rd.
        printf("es un ADDS extended!!!!\n");
        //Rd
        uint32_t Rd_mask = 0b11111;
        uint32_t Rd = (instruction & Rd_mask);
        //Rn
        uint32_t Rn_mask = 0b11111<<5;
        uint32_t Rn = (instruction & Rn_mask)>>5;
        //imm12
        uint32_t imm3_mask = 0b111<<10;
        uint32_t imm3 = (instruction & imm3_mask)>>10;
        //option
        uint32_t option_mask = 0b111>>13;
        uint32_t option = (instruction & option_mask)<<13;
        //Rm
        uint32_t Rm_mask = 0b11111>>16;
        uint32_t Rm = (instruction & Rm_mask)<<16;

        //Operando 1
        if (registers[Rn] == 31){
            Rn = CURRENT_STATE.REGS[31];
        } else {
            Rn = CURRENT_STATE.REGS[Rn];
        }
        //Operando 2
        Rm = CURRENT_STATE.REGS[Rm];
        switch  (option){
            case 0b000:
                Rm = (uint8_t)Rm;
                break;
            case 0b001:
                Rm = (uint16_t)Rm;
                break;
            case 0b010:
                Rm = (uint32_t)Rm;
                break;
            case 0b011:
                Rm = (uint64_t)Rm;
                Rm = Rm << imm3;
                break;
            case 0b100:
                Rm = (int8_t)Rm;
                break;
            case 0b101:
                Rm = (int16_t)Rm;
                break;
            case 0b110:
                Rm = (int32_t)Rm;
                break;
            case 0b111:
                Rm = (uint64_t)Rm;
                break;
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
        NEXT_STATE.REGS[Rd] = res;
        NEXT_STATE.FLAG_N = (res < 0);
        NEXT_STATE.FLAG_Z = (res == 0);
    }

    if ((instruction & mask_8bits) == adds_immediate_opcode) {
        printf("es un ADDS immediate!!!!\n");
        //Rd
        uint32_t Rd_mask = 0b11111;
        uint32_t Rd = instruction & Rd_mask;
        printf("Rd: %d\n", Rd);
        //Rn
        uint32_t Rn_mask = 0b11111<<5;
        uint32_t Rn = (instruction & Rn_mask)>>5;
        printf("Rn: %d\n", Rn);
        //imm12
        uint32_t imm12_mask = 0b111111111111<<10;
        uint32_t imm12 = (instruction & imm12_mask)>>10;
        printf("imm12: %d\n", imm12);
        //shift
        uint32_t shift_mask = 0b11<<22;
        uint32_t shift = (instruction & shift_mask)>>22;
        printf("shift: %d\n", shift);
        if (shift == 01){
            imm12 = imm12<<12;
        }
        //Operacion y flags
        uint64_t res = registers[Rn] + imm12;
        printf("res: %ld\n", res);
        //mem_write_32(Rd, res);
        //registers[Rd] = res;
        NEXT_STATE.REGS[Rd] = res;
        NEXT_STATE.FLAG_N = (res < 0);
        NEXT_STATE.FLAG_Z = (res == 0);
    }

    //Actualizo PC
    NEXT_STATE.PC += 4;
}