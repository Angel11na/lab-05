//#pragma once
#ifndef _CPU_HPP_
#define _CPU_HPP_

#include <iostream>
#include <cstdint>
using namespace std;

#include "memory.hpp"

struct DecodedOp {
    Byte high;  // Старший півбайт
    Byte low;   // Молодший півбайт 
};
DecodedOp decode(Byte op);


const Byte GRP_IO       = 0x0;
const Byte GRP_ALU      = 0x1;
const Byte GRP_DATA     = 0x2;
const Byte GRP_JUMP     = 0x3;
const Byte GRP_DISPLAY  = 0x4;

//  --- IO
const Byte OP_HALT = 0x0;  //  Stop VM (exit)   0
const Byte OP_NOP  = 0x1;  //  No operation     1
const Byte OP_OUT  = 0x2;  //  Print A as symbol
const Byte OP_OUTN = 0x3;  //  Print A as DEC number +space 
const Byte OP_OUTS = 0x04; //  Print bytes from Addr while not '\0'
//  --- ALU
const Byte OP_ADD  = 0x0;  //  +    16
const Byte OP_SUB  = 0x1;  //  -    17
const Byte OP_AND  = 0x2;  //  &&   18
const Byte OP_OR   = 0x3;  //  ||   19
const Byte OP_XOR  = 0x4;  //  %2   20
const Byte OP_NOT  = 0x5;  //  !    21
const Byte OP_SHL  = 0x6;  //  <<   22
const Byte OP_SHR  = 0x7;  //  >>   23
const Byte OP_INC  = 0x8;  //  ++   24
const Byte OP_DEC  = 0x9;  //  --   25
const Byte OP_CMP  = 0xa;  //  A-B, set Z/N/C
//  --- DATA
const Byte OP_LOADI_A           = 0x0;  //  Load 8 bits to A
const Byte OP_LOADI_B           = 0x1;  //  Load 8 bits to B
const Byte OP_LOAD_A            = 0x2;  //  Load 8 bits to A from mem[addr16]
const Byte OP_LOAD_B            = 0x3;  //  Load 8 bits to B from mem[addr16]
const Byte OP_STORE_A           = 0x4;  //  Put A to mem[addr16]
const Byte OP_STORE_B           = 0x5;  //  Put B to mem[addr16]
const Byte OP_MOV_AB            = 0x6;  //  A = B
const Byte OP_MOV_BA            = 0x7;  //  B = A
const Byte OP_LOADH             = 0x8;  //  Load 16 bit to H
const Byte OP_LOAD_AH           = 0x9;  //  A = mem[H]
const Byte OP_STORE_HA          = 0xa;  //  mem[H] = A
const Byte OP_INC_H             = 0xb;  //  H +1
const Byte OP_DEC_H             = 0xc;  //  H -1
const Byte OP_H_LOW             = 0xd;  //  low byte from H
//  --- JUMP
const Byte OP_JMP               = 0x0;  // jump anyway
const Byte OP_JZ                = 0x1;  // jump, if Z=1
const Byte OP_JNZ               = 0x2;  // jump, if Z=0
const Byte OP_JC                = 0x3;  // jump, if C=1
const Byte OP_JNC               = 0x4;  // jump, if C=0
// --- DISPLAY
const Byte OP_CLS               = 0x0;
const Byte OP_PLOT              = 0x1;
const Byte OP_UNPLOT            = 0x2;
const Byte OP_SHOW              = 0x3;








// Коди операцій — саме з ISA.uk.md, свого не вигадуйте.





//  ---------------------------------------------------------



//  Types
//
struct Flags {
    bool z = false;   // результат нульовий
    bool n = false;   // старший біт результату — одиниця
    bool c = false;   // біт, який виїхав за межу восьми
};

struct CPU {
    Memory* mem = nullptr;   // вказівник на пам'ять
    U16 pc = 0;     //  адреса поточноъ інструкції для step
    U16 H = 0;      //  пам'ять для адреси
    Byte a = 0;
    Byte b = 0;
    Flags f;
    bool halted = false;     // після HALT наступні step відмовляють
};
//  -----------------------------------------------------------------



//  Functions
//  
void step(CPU& cpu);
void dump_regs(const CPU& cpu);
string flags_state(const CPU& cpu);
//  ----------------------------------------

#endif