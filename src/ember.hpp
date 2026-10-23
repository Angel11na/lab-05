#ifndef _EMBER_HPP_
#define _EMBER_HPP_

#include <iostream>
#include <sstream>
using namespace std;

#include "cpu.hpp"
#include "dump.hpp"

//  Command functions
void cmd_get(Memory& mem, istringstream& words);
void cmd_set(Memory& mem, istringstream& words);
void cmd_inc(Memory& mem, istringstream& words);
void cmd_reg(CPU& cpu, istringstream& words);
void cmd_get16(Memory& mem, istringstream& words);
void cmd_set16(Memory& mem, istringstream& words);
void cmd_run(CPU &cpu);

void cmd_find(Memory& mem, istringstream& words);

#endif