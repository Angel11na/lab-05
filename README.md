# M1 — обробка невідомих опкодів

Додано у `cpu.cpp`:
```cpp
static void unknown_opcode(Byte opcode, CPU &cpu){
    cerr << "Unknown opcode 0x" << hex << static_cast<int>(opcode)
        << " at PC=0x" << cpu.pc << dec << "\n";
    cpu.halted = true;
}
```
Функція викликається для `default:` всіх груп опкодів.

Перевірка роботи:
```
ember> set 0 0x04
ember> step
Unknown opcode 0x4 at PC=0x0
ember> step
halted
ember>
``` 



# M2 — стрибки. 
    
Група `0x3_` з *ISA.uk.md* плюс `CMP` (`0x1A`):

**CMD** | **Type**    |   **Пояснення**
--------|-------------|---------------------------------------------------------------------------
`JMP`   | `addr16`    | `PC = addr`. І не додавати після цього ще 3. Це найчастіший баг цієї лаби
`JZ`    | `addr16`    | якщо `Z`, то `PC = addr`, інакше `PC += 3`
`JNZ`   | `addr16`    | навпаки
`CMP`   | `A`, `B`    | порахувати `A - B`, виставити `Z`/`N`/`C`, результат викинути

Для реалізації перевірки флагів, написано загальну ф-ю:
```cpp
//  загальна ф-я для обробки стрибків за умови
static void JMPX(bool flag, CPU &cpu){
    if(!flag){   //  якщо флаг НЕ виставлений - пропустити цю команду
        cpu.pc +=3;
        return;
    }
    //  інакше, флаг ВИСТАВЛЕНИЙ
    cpu.pc = get16(*cpu.mem, cpu.pc+1);
}
```




Ліміт кроків для команди `run`:
```cpp
void cmd_run(CPU& cpu) {
    const size_t MAX_STEPS = 100'000;
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
```

Для тесту, ліміт кроків встановлено `MAX_STEPS = 1`:
```
ember 0.1 - 4096 bytes of memory you can see. Type `help`.
ember> set 0 0x01
ember> set 1 0x02
ember> run
MAX_STEPS (1) limit reached, run stopped
```


# M3 — цикл у байткоді 

## Умова
Зворотний відлік: 
- покладіть сім байтів, 
- зробіть run, 
- отримайте 3 2 1. 

адр. | байти    |  текст
-----|----------|------------------
0000 | 20 03    | LOADI A, 3
0002 | 03       | loop:  OUTN
0003 | 19       |        DEC A
0004 | 32 02 00 |        JNZ loop
0007 | 00       |        HALT


## Код для копіювання в **ember**:
```
set 0 0x20
set 1 0x03
set 2 0x03
set 3 0x19
set 4 0x32
set 5 0x02
set 6 0x00
set 7 0x00
```

## Виведення:
```
ember> set 0 0x20
set 1 0x03
set 2 0x03
set 3 0x19
set 4 0x32
set 5 0x02
set 6 0x00
set 7 0x00
ember> 
ember> 
ember> 
ember> 
ember> 
ember> 
ember> 

ember> run
3 2 1 
```

## Трасування


```
Крок    | Значення          | С++                       | Пояснення
---------------------------------------------------------------------------------------------
1       | PC=0x00 A=0 Z=0   | int a = 3;                | Оголошення A (ініціалізація)
2       | PC=0x02 A=3 Z=0   | do { cout << a << " ";    | Цикл з післ.умовою: { виведення А
3       | PC=0x03 A=3 Z=0   |   a--;                    |   Декремент А
4       | PC=0x04 A=2 Z=0   | } while(a != 0);          | } Доки (А != 0)
5       | PC=0x02 A=2 Z=0   | // cout << a << " ";      | У циклі: -- Виведення А
6       | PC=0x03 A=2 Z=0   | // a--;                   | У циклі: -- Декремент А
7       | PC=0x04 A=1 Z=0   | // while(a != 0);         | У циклі: -- Перевірка умови
8       | PC=0x02 A=1 Z=0   | // cout << a << " ";      | У циклі: -- Виведення А
9       | PC=0x03 A=1 Z=0   | // a--;                   | У циклі: -- Декремент А
10      | PC=0x04 A=0 Z=1   | // while(a != 0);         | У циклі: -- Перевірка умови
11      | PC=0x07 A=0 Z=1   | return;                   | Зупинка (return, вихід)
```




# M4 — лінійний пошук, двічі


### Код команди описано в файлі `ember.cpp`:
```cpp
//  find <lo> <hi> <byte>
//  <lo> <hi> - це адреса, з якої почати пошук (глибиною 8 байт)
//  <byte> - яке значення шукаємо?
void cmd_find(Memory& mem, istringstream& words){
    string l, h, b;
    long low, high, search_byte;

    if( //  зчитування слів, перетворення у числа
        !(words >> l) || !(words >> h) || !(words >> b) ||
        !parse_number(l, low) || !parse_number(h, high) || !parse_number(b, search_byte)
    ) {
        cout << "usage: find <lo> <hi> <byte>\n";
        return;
    }



    //  збірка 16-адреси з двох байтів (8)
    U16 addr = 0;
    addr = addr | low;
    addr = addr | (high << 8);

    //  перетворення шуканого байту у число
    long byte_value = static_cast<long>(search_byte);
    if(byte_value < 0 || byte_value > 255) {    //  перевірка меж
        cout << "address must not be 0-255, got " << byte_value << "\n";
        return;
    }

    bool finded = false;    //  чи знайдено хоча б один збіг?

    //  цикл пошуку: для addr та наступних 7 адрес
    for(int i = 0; i < 8; i++){
        Byte mem_value = mem.data[addr + i];    //  взяти значення

        //  якщо співпало
        if(static_cast<long>(mem_value) == search_byte) {
            cout << "Value 0x" << hex << search_byte << " was finded at address 0x" << (addr+i) << dec << "\n";
            finded = true;
        }
    }

    //  якщо жодного збігу
    if(!finded) {
        cout << "Value 0x" << hex << search_byte << " was NOT finded at 0x" << addr << " - 0x" << addr+8 << dec << "\n";
    }
}
```

### Тестування команди `find`:
```
ember 0.1 - 4096 bytes of memory you can see. Type `help`.

ember> set 0x0804 0x41

ember> set 0x0805 0x15

ember> set 0x0806 0x15

ember> find 0x00 0x08 0x41
Value 0x41 was finded at address 0x804

ember> find 0x00 0x08 0x15
Value 0x15 was finded at address 0x805
Value 0x15 was finded at address 0x806

ember> find 0x00 0x08 0x67
Value 0x67 was NOT finded at 0x800 - 0x808

ember> 
```