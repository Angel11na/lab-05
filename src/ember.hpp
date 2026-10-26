#ifndef _EMBER_HPP_
#define _EMBER_HPP_

#include <iostream>
#include <sstream>
using namespace std;

#include "cpu.hpp"
#include "dump.hpp"


const size_t MAX_STEPS = 100'000;

//  Command functions
void cmd_get(Memory& mem, istringstream& words);
void cmd_set(Memory& mem, istringstream& words);
void cmd_inc(Memory& mem, istringstream& words);
void cmd_reg(CPU& cpu, istringstream& words);
void cmd_get16(Memory& mem, istringstream& words);
void cmd_set16(Memory& mem, istringstream& words);
void cmd_run(CPU &cpu);

void cmd_find(Memory& mem, istringstream& words, bool first_only = false);


void cmd_draw_frame(Memory &mem);


#endif