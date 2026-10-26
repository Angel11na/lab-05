#ifndef _DISPLAY_HPP_
#define _DISPLAY_HPP_

#include "memory.hpp"

namespace display {

    const U16 VRAM_LO = 0x0A00;

    const int VRAM_ROWS = 32;
    const int VRAM_COLS = 64;

    const char VRAM_SYMBOL_FILL = '#';      // 
    const char VRAM_SYMBOL_EMPTY = '.';     // 


    void show(const Memory& mem);
    void plot(Memory& mem, int x, int y);
    void unplot(Memory& mem, int x, int y);
    void clear(Memory& mem);
}

#endif