#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

#include <iostream>
#include <cstddef>
#include <cstdint>

using namespace std;

// --- ALIASES
using Byte = uint8_t;
using U16 = uint16_t;

// `const` means: this name will not be used to change these bits.
const size_t MEM_SIZE = 4096;


struct Memory {
    // The `{}` zero-initializes the whole array.
    // Experiment (Lab 1, M4): delete the `{}`, rebuild, `dump`. What appears?
    // Put the `{}` back afterwards — reading uninitialized memory is UB,
    // and this course does not ship UB.
    Byte data[MEM_SIZE]{};
};

// Read the byte at `addr`. If `addr` is outside the box, return 0.
// TODO(lab-01, M3): implement in memory.cpp.
Byte mem_get(const Memory& mem, size_t addr);
U16 get16(const Memory& mem, size_t addr);

// Write `value` at `addr`. Return false (and change nothing) if `addr` is
// outside the box. Returning false is how main.cpp knows to print a message.
// TODO(lab-01, M3): implement in memory.cpp.
bool mem_set(Memory& mem, size_t addr, Byte value);
bool set16(Memory& mem, size_t addr, U16 value);

//  Get byte, increment and overwrite
bool inc(Memory& mem, size_t addr);

#endif