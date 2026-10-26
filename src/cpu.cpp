#include <iomanip>
#include <iostream>
using namespace std;

#include "alu.hpp"
#include "display.hpp"
#include "cpu.hpp"
#include "dump.hpp"

DecodedOp decode(Byte op) {
    return {
        .high = (Byte)((op & 0xF0) >> 4), // Старші 4 біти
        .low = (Byte)(op & 0x0F)          // Молодші 4 біти
    };
}


static bool is_mem_end(CPU &cpu) {
    size_t idx = static_cast<size_t>(cpu.pc);
    if(idx >= MEM_SIZE){
        cout << "cpu.pc >= " << MEM_SIZE << ", value was " << cpu.pc << ". CPU was halted.\n";
        cpu.halted = true;
        return true;
    }
    return false;
}

// універсальний обробник невідомого опкоду для всіх default:
static void unknown_opcode(Byte opcode, CPU &cpu){
    cerr << "Unknown opcode 0x" << hex << static_cast<int>(opcode)
        << " at PC=0x" << cpu.pc << dec << "\n";
    cpu.halted = true;
}

//  обробка оп-кодів групи IO
static void handle_group_io(Byte op, CPU &cpu){
    switch (op) {
        case OP_HALT:
            cpu.halted = true;
            cpu.pc++; // розмір HALT з таблиці — 1
            break;
        case OP_NOP:
            cpu.pc++;
            break;
        case OP_OUT:
            cout << to_character(cpu.a) << "\n";
            cpu.pc++;
            break;
        case OP_OUTN:
            cout << static_cast<int>(cpu.a) << " ";
            cpu.pc++;
            break;
        case OP_OUTS: {
            cpu.pc++;
            U16 addr = get16(*cpu.mem, cpu.pc);
            cpu.pc+=2;

            for(int i = 0; i < 256; i++){
                if(cpu.mem->data[addr + i] == '\0')
                    break;
                
                cout << static_cast<char>(cpu.mem->data[addr+i]);
            }

            break;
        }
        default:
            unknown_opcode(op, cpu);
            break;
    }
}

//  обробка оп-кодів групи DATA
static void handle_group_data(Byte op, CPU &cpu){
    switch (op) {
        case OP_LOADI_A: {
            cpu.pc++;                           //  наступна комірка = значення, imm8
            if(is_mem_end(cpu)) return;
            cpu.a = mem_get(*cpu.mem, cpu.pc);  //  взяти значення з пам'яті
            cpu.pc++;                           //  наступна інструкція
            break;
        }
        case OP_LOADI_B: {
            cpu.pc++;                           //  наступна комірка = значення, imm8
            if(is_mem_end(cpu)) return;
            cpu.b = mem_get(*cpu.mem, cpu.pc);  //  взяти значення з пам'яті
            cpu.pc++;                           //  наступна інструкція
            break;
        }
        case OP_LOAD_A: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            cpu.a = mem_get(*cpu.mem, addr);        //  у регістр А
            cpu.pc+=2;
            break;
        }
        case OP_LOAD_B: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            cpu.b = (Byte)mem_get(*cpu.mem, addr);        //  у регістр B
            cpu.pc+=2;
            break;
        }
        case OP_STORE_A: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            mem_set(*cpu.mem, addr, cpu.a);
            cpu.pc+=2;
            break;
        }
        case OP_STORE_B: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            mem_set(*cpu.mem, addr, cpu.b);
            cpu.pc+=2;
            break;
        }
        case OP_MOV_AB: {
            cpu.a = cpu.b;
            cpu.pc++;
            break;
        }
        case OP_MOV_BA: {
            cpu.b = cpu.a;
            cpu.pc++;
            break;
        }
        case OP_LOADH: {
            cpu.pc++;
            if(is_mem_end(cpu)) return;
            U16 addr = get16(*cpu.mem, cpu.pc);
            cpu.H = addr;
            cpu.pc+=2;
            break;
        }
        case OP_LOAD_AH: {
            cpu.a = mem_get(*cpu.mem, cpu.H);
            cpu.pc++;
            break;
        }
        case OP_STORE_HA: {
            mem_set(*cpu.mem, cpu.H, cpu.a);
            cpu.pc++;
            break;
        }
        case OP_INC_H: {
            cpu.H++;
            cpu.pc++;
            break;
        }
        case OP_DEC_H: {
            cpu.H--;
            cpu.pc++;
            break;
        }
        case OP_H_LOW: {
            /* NOT THIS LAB */
            break;
        }
        default: {
            unknown_opcode(op, cpu);
            break;
        }
    }
}

//  обробка оп-кодів групи ALU
static void handle_group_alu(Byte op, CPU &cpu){
    switch (op) {
        case OP_ADD: {
            cout << "\n"
                 << "ADD(" << static_cast<int>(cpu.a) << ", " << static_cast<int>(cpu.b)
                 << ") = " << static_cast<int>(ADD(cpu.a, cpu.b, cpu.f)) << flags_state(cpu)
                 << "\n\n";
            cpu.pc++;
            break;
        }

        case OP_SUB: {
            cout << "\n"
                 << "SUB(" << static_cast<int>(cpu.a) << ", " << static_cast<int>(cpu.b)
                 << ") = " << static_cast<int>(SUB(cpu.a, cpu.b, cpu.f)) << flags_state(cpu)
                 << "\n\n";
            cpu.pc++;
            break;
        }

        case OP_AND: {
            cout << "\n"
                 << "AND = " << to_bin(AND(cpu.a, cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case OP_OR: {
            cout << "\n"
                 << "OR = " << to_bin(OR(cpu.a, cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case OP_XOR: {
            cout << "\n"
                 << "XOR = " << to_bin(XOR(cpu.a, cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case OP_NOT: {
            cout << "\n"
                 << "NOT(a) = " << to_bin(NOT(cpu.a, cpu.f)) << flags_state(cpu) << "\n"
                 << "NOT(b) = " << to_bin(NOT(cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case OP_SHL: {
            cout << "\n"
                 << "SHL(a) = " << to_bin(SHL(cpu.a, cpu.f)) << flags_state(cpu) << "\n"
                 << "SHL(b) = " << to_bin(SHL(cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case OP_SHR: {
            cout << "\n"
                 << "SHR(a) = " << to_bin(SHR(cpu.a, cpu.f)) << flags_state(cpu) << "\n"
                 << "SHR(b) = " << to_bin(SHR(cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            cpu.pc++;
            break;
        }

        case OP_INC: {
            /*cout << "\n"
                 << "INC(a) = " << to_bin(INC(cpu.a, cpu.f)) << flags_state(cpu) << "\n"
                 << "INC(b) = " << to_bin(INC(cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            */
            cpu.a = INC(cpu.a, cpu.f);
            cpu.pc++;
            break;
        }

        case OP_DEC: {
            /*cout << "\n"
                 << "DEC(a) = " << to_bin(DEC(cpu.a, cpu.f)) << flags_state(cpu) << "\n"
                 << "DEC(b) = " << to_bin(DEC(cpu.b, cpu.f)) << flags_state(cpu) << "\n\n";
            */
            cpu.a = DEC(cpu.a, cpu.f);
            cpu.pc++;
            break;
        }

        case OP_CMP: {
            (void)SUB(cpu.a, cpu.b, cpu.f);
            cpu.pc++;
            break;
        }

        default: {
            unknown_opcode(op, cpu);
            break;
        }
    }
}

//  обробка оп-кодів групи DISPLAY
static void handle_group_display(Byte op, CPU &cpu){
    switch(op){
        case OP_SHOW:
            display::show(*cpu.mem);
            cpu.pc++;
            break;

        case OP_PLOT:
            display::plot(*cpu.mem, cpu.a, cpu.b);
            cpu.pc++;
            break;

        case OP_UNPLOT:
            display::unplot(*cpu.mem, cpu.a, cpu.b);
            cpu.pc++;
            break;

        case OP_CLS:
            display::clear(*cpu.mem);
            cpu.pc++;
            break;

        default:
            unknown_opcode(op,cpu);
            break;
    }
}

//  загальна ф-я для обробки стрибків за умови
static void JMPX(bool flag, CPU &cpu){
    if(!flag){   //  якщо флаг НЕ виставлений - пропустити цю команду
        cpu.pc +=3;
        return;
    }
    //  інакше, флаг ВИСТАВЛЕНИЙ
    cpu.pc = get16(*cpu.mem, cpu.pc+1);
}

//  обробка оп-кодів групи JUMP
static void handle_group_jump(Byte op, CPU &cpu){
    switch (op) {
        case OP_JMP:
            cpu.pc = get16(*cpu.mem, cpu.pc+1);
            break;

        case OP_JZ:
            JMPX(cpu.f.z, cpu);
            break;

        case OP_JNZ:
            JMPX(!cpu.f.z, cpu);
            break;

        case OP_JC:
            JMPX(cpu.f.c, cpu);
            break;

        case OP_JNC:
            JMPX(!cpu.f.c, cpu);
            break;

        default:
            unknown_opcode(op, cpu);
            break;
    }
}



void step(CPU& cpu) {
    if (cpu.halted) {
        cout << "halted\n";
        return;
    }
    //  -- cpu.mem - це сам вказівник
    //  ~ cpu.mem = ...                                         | переміщення
    //  ~ cpu.mem++ / cpu.mem-- / cpu.mem += ... / cpu.mem -=   | переміщення через арифметику
    //  ~ *cpu.mem                                              | взяти значення
    Byte op = mem_get(*cpu.mem, cpu.pc);

    DecodedOp dop = decode(op);

    //  ---------------  TEST ---------------------
    //dump_regs(cpu);
    //  -------------------------------------------

    switch (dop.high) {
        case GRP_IO:
            handle_group_io(dop.low, cpu);
            break;
        
        case GRP_ALU:
            handle_group_alu(dop.low, cpu);
            break;
        
        case GRP_DATA:
            handle_group_data(dop.low, cpu);
            break;
        
        case GRP_JUMP:
            handle_group_jump(dop.low, cpu);
            break;
        
        case GRP_DISPLAY:
            handle_group_display(dop.low, cpu);
            break;

        default:
            unknown_opcode(op, cpu);
            break;
    }
}

void dump_regs(const CPU& cpu) {
    cout << "PC = " << "0x" << hex << cpu.pc << dec << "\n";
    cout << "H = " << "0x" << hex << setw(2) << setfill('0') << cpu.H << dec << "  "
         << to_bin(cpu.H, 16, 4) << "\n";
    cout << "A = ";
    show_byte(cpu.a);
    cout << "B = ";
    show_byte(cpu.b);
    cout << "Z: " << cpu.f.z << "\n" << "N: " << cpu.f.n << "\n" << "C: " << cpu.f.c << "\n";
}

inline const char* bit(bool b) { return b ? "1" : "0"; }
string flags_state(const CPU& cpu) {
    return "   |   Z: " + string(bit(cpu.f.z)) + "  " + "N: " + string(bit(cpu.f.n)) + "  " +
           "C: " + string(bit(cpu.f.c));
}