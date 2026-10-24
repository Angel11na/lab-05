#include "ember.hpp"
#include "display.hpp"

// Turn a word into a number. Accepts decimal (65) and hex (0x41).
// Returns false if the word is not a number at all.
static bool parse_number(const string& word, long& out) {
    try {
        size_t used = 0;
        // base 0 means: look at the prefix. "0x41" is hex, "65" is decimal.
        out = stol(word, &used, 0);
        return used == word.size(); // reject things like "12abc"
    } catch (...) {
        return false;
    }
}

//  =============================================================================
//  ==============  COMMANDS
//
void cmd_get(Memory& mem, istringstream& words) {
    string a;
    long addr = 0;
    if (!(words >> a) || !parse_number(a, addr)) {
        cout << "usage: get <addr>\n";
        return;
    }
    if (addr < 0) {
        cout << "address must not be negative\n";
        return;
    }
    
    show_byte(mem_get(mem, static_cast<size_t>(addr)));
}

void cmd_set(Memory& mem, istringstream& words) {
    string a, v;
    long addr = 0, value = 0;
    if (!(words >> a) || !(words >> v) || !parse_number(a, addr) || !parse_number(v, value)) {
        cout << "usage: set <addr> <value>\n";
        return;
    }
    if (addr < 0) {
        cout << "address must not be negative\n";
        return;
    } 
    if (value < 0 || value > 255) {
        // A cell holds ONE byte. 256 does not fit. Lab 1, theory 3.
        cout << "a byte is 0..255, got " << value << '\n';
        return;
    }
    if (!mem_set(mem, static_cast<size_t>(addr), static_cast<Byte>(value))) {
        cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1 << '\n';
        return;
    }
}

void cmd_inc(Memory& mem, istringstream& words) {
    string a;
    long addr = 0;

    if (!(words >> a) || !parse_number(a, addr)) {
        cout << "usage: inc <addr>\n";
    } else if (addr < 0) {
        cout << "address must not be negative\n";
    } else {
        bool result = inc(mem, addr);

        if (!result) {
            cout << "Error during cmd: inc <addr>, addr = " << addr << "\n";
        }
    }
}

void cmd_reg(CPU& cpu, istringstream& words) {
    string r, v;
    long value = 0;
    
    if (
        !(words >> r) || !(words >> v) ||   //  чи достатньо елементів у команді?
        !(r == "a" || r == "b") ||          //  r є "а" або "b"?
        !parse_number(v, value) ||          //  чи v є цілим числом?
        value < 0 || value > 255            //  чи v в межах 0-255
    ) {
        cout << "usage: reg <a|b> <0..255>\n";
        return;
    }
    
    cpu.a = (r == "a") ? (Byte)value : cpu.a;
    cpu.b = (r == "b") ? (Byte)value : cpu.b;
}


void cmd_get16(Memory& mem, istringstream& words){
    string a;
    long addr = 0;
    if (!(words >> a) || !parse_number(a, addr)) {
        cout << "usage: get16 <addr>\n";
        return;
    }
    if (addr < 0) {
        cout << "address must not be negative\n";
        return;
    }
    U16 result = get16(mem, addr);

    show_two_bytes(result);
}


void cmd_set16(Memory& mem, istringstream& words){
    string a, v;
    long addr = 0, value = 0;
    if (!(words >> a) || !(words >> v) || !parse_number(a, addr) || !parse_number(v, value)) {
        cout << "usage: set16 <addr> <value>\n";
        return;
    }
    if (addr < 0) {
        cout << "address must not be negative\n";
        return;
    } 
    if (value < 0 || value > 65'535) {
        // A cell holds TWO byte. Value less than 65536 (2^16) 
        cout << "a byte is 0..65535, got " << value << '\n';
        return;
    }

    if(!set16(mem, addr, static_cast<U16>(value))){
        cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1 << '\n';
    }
}





void cmd_run(CPU& cpu) {
    size_t steps = 0;

    if (cpu.halted) {
        cerr << "Error: CPU halted\n";
        return;
    }

    while (!cpu.halted) {

        if(steps >= MAX_STEPS){
            cout << "MAX_STEPS (" << MAX_STEPS << ") limit reached, run stopped\n";
            break;
        }

        step(cpu);
        steps++;
    }   
}

//  find <lo> <hi> <byte> <?deepth=8>
//  <lo> <hi> - це адреса, з якої почати пошук (глибиною 8 байт)
//  <byte> - яке значення шукаємо?
void cmd_find(Memory& mem, istringstream &words, bool first_only){
    string l, h, b, d;
    long low, high, search_byte;
    int deepth = 8;

    if( //  зчитування слів, перетворення у числа
        !(words >> l) || !(words >> h) || !(words >> b) ||
        !parse_number(l, low) || !parse_number(h, high) || !parse_number(b, search_byte)
    ) {
        cout << "usage: find <lo> <hi> <byte> <?deepth=8>\n";
        return;
    }

    long ldeepth = 0;       //  зчитування глибини пошуку (опціонально)
    if(words >> d) {
        parse_number(d, ldeepth);       //  перетворення у число

        if(ldeepth > 0 && (size_t)ldeepth < MEM_SIZE){
            //  якщо більше 0 та менше за розмір пам'яті - прийняти значення
            deepth = ldeepth;
        }
    }

    //  збірка 16-адреси з двох байтів (8)
    U16 addr = 0;
    addr = addr | low;
    addr = addr | (high << 8);

    //  перетворення шуканого байту у число
    long byte_value = static_cast<long>(search_byte);
    if(byte_value < -1 || byte_value > 255) {    //  перевірка меж
        cout << "byte must not be 0-255 of -1 (non-zero search), got " << byte_value << "\n";
        return;
    }

    bool finded = false;    //  чи знайдено хоча б один збіг?

    //  цикл пошуку: для addr та наступних deepth адрес
    for(int i = 0; i < deepth; i++){    //  до глибини пошуку
        if((size_t)(addr + i) >= MEM_SIZE)        //  чи кінець пам'яті?
            break;

        Byte mem_value = mem.data[addr + i];    //  взяти значення

        //  якщо співпало
        if(
            (static_cast<long>(mem_value) == search_byte) ||            //  збіг
            (search_byte == -1 && static_cast<long>(mem_value) != 0)    //  ненульове значення, якщо пошук за -1
        
        ) {
            cout << "Value 0x" << hex << static_cast<long>(mem_value) << " was finded at address 0x" << (addr+i) << dec << "\n";
            finded = true;

            if(first_only)
                return;
        }
    }

    //  якщо жодного збігу
    if(!finded) {
        cout << "Value 0x" << hex << search_byte << " was NOT finded at 0x" << addr << " - 0x" << addr+deepth << dec << "\n";
    }
}

void draw_frame(Memory &mem){
    display::clear(mem);
    for(int i = 0; i < display::VRAM_COLS; i++)
        display::plot(mem, i, 0);
    //  нижня
    for(int i = 0; i < display::VRAM_COLS; i++)
        display::plot(mem, i, display::VRAM_ROWS-1);
    //  ліва
    for(int i = 0; i < display::VRAM_ROWS; i++)
        display::plot(mem, 0, i);
    //  права
    for(int i = 0; i < display::VRAM_ROWS; i++)
        display::plot(mem, display::VRAM_COLS-1, i);
}