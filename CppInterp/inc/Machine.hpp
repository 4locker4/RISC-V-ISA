#pragma once

#include <cstdint>    // for uint32_t, uint8_t
#include <cstring>    // for size_t, memcpy
#include <fstream>    // for basic_ostream, operator<<, basic_ifstream, endl
#include <iomanip>    // for operator<<, setfill, setw
#include <iostream>   // for cout, cerr
#include <stdexcept>  // for out_of_range, runtime_error
#include <string>     // for char_traits, basic_string, string
#include <vector>     // for vector
#include <list>       // for list

#include "./Enums.hpp"

using namespace std;

namespace Interpretator{

    
    typedef uint32_t command_t;
    typedef uint32_t reg_t;
    typedef uint32_t addr_t;
    typedef uint8_t mem_t;
    
    static const unsigned int N_REGISTERS = 32;
    static const unsigned int N_MEM = 1024;
    static const unsigned int TERMINAL_VADDR = 0x10;
    
    struct SEG_VADDR_DATA{
         uint32_t startVaddr = 0;
         uint32_t endVaddr = 0;
         uint8_t  flags = 0;
 
         SEG_VADDR_DATA(uint32_t startVaddrGet, uint32_t endVaddrGet, uint8_t flagsGet)
             : startVaddr(startVaddrGet), endVaddr(endVaddrGet), flags(flagsGet) {};
     };

    typedef struct Command_st {
        reg_t rd : 5;
        reg_t rs : 5;
        reg_t rt : 5;
        uint32_t imm;
    } Command_st;

    class CPU_State {
        reg_t registers[N_REGISTERS] = {0};
        reg_t PC = 0;

    public:

        reg_t GetReg(uint32_t regNum) { return registers[regNum]; }
        void SetReg(uint32_t regNum, reg_t value) { registers[regNum] = value; }
    
        reg_t GetPC() { return PC; }
        void IncrPC(uint32_t value) { PC += value; }
        void SetPC(reg_t value) { PC = value; }

        void DumpRegisters() const {
            std::cout << "CPU Registers:\n";
            for (int i = 0; i < N_REGISTERS; ++i) {
                std::cout << "  x" << std::setw(2) << std::setfill(' ') << i 
                        << ": 0x" << std::hex << std::setw(8) << std::setfill('0') 
                        << registers[i] << " (" << std::dec << registers[i] << ")\n";
            }
            std::cout << "  PC : 0x" << std::hex << std::setw(8) << std::setfill('0') 
                    << PC << " (" << std::dec << PC << ")\n";
            std::cout << std::flush;
        }
   };

    class Memory{
        vector<mem_t> memory;
    
        public:
    
        uint32_t ReadWord(reg_t index) const {
            if (index % sizeof(command_t) != 0) {
                throw std::runtime_error("ReadWord: misaligned access");
            }
        
            if (index + sizeof(mem_t) > memory.size()) {
                throw std::out_of_range("ReadWord: address out of range");
            }
        
            uint32_t value;
            memcpy(&value, memory.data() + index, sizeof(uint32_t));
            return value;
        }

        void WriteFromFile(size_t memAlign, std::ifstream &file, size_t fileAlign, size_t sizeInFile){
            if (memAlign % sizeof(command_t) != 0) {
                throw std::runtime_error("WriteFromFile: misaligned access");
            }

            if (memAlign + sizeInFile + sizeof(mem_t) > memory.size()) {
                memory.resize(memAlign + sizeInFile + sizeof(mem_t));
            }

            file.seekg(fileAlign);
            file.read(reinterpret_cast<char *> (memory.data() + memAlign), sizeInFile);
        }
    
        void WriteWord(reg_t index, reg_t value) {
            if (index % sizeof(command_t) != 0) {
                throw std::runtime_error("ReadWord: misaligned access");
            }

            if (index + sizeof(mem_t) > memory.size()) {
                memory.resize(index + sizeof(mem_t));
            }
        
            mem_t* ptr = memory.data() + index * sizeof(mem_t);
            memcpy(ptr, &value, sizeof(reg_t));
        }
    
        void MemOut() {
            cout << "Memory dump:" << endl;
            for (size_t i = 0; i < memory.size(); i += 32) {
                cout << hex << setw(8) << setfill('0') << i << ": ";
        
                for (size_t j = 0; j < 8; j++) {
                    size_t byte_offset = i + j * sizeof(uint32_t);
                    if (byte_offset + sizeof(uint32_t) <= memory.size()) {
                        uint32_t word = *reinterpret_cast<const uint32_t*>(
                            memory.data() + byte_offset
                        );
                        cout << hex << setw(8) << setfill('0') << word << " ";
                    } else {
                        cout << "-------- ";
                    }  
                }
                cout << dec << endl;
            }
        }
    };

/*
    Можно сделать некоторый менеджер виртуальной памяти, чтобы тот следил за тем, что виртуальной памяти хватает, выделял новую и тд.
    Но для данной задачи - оверинжениринг. Поэтому будет отдельной веткой, ближе к виртуальной машине. Пока что, для упрощения кода
    адрес виртуальной памяти будет = "физическому" адресу. Из-за этого количество выделяемой памяти про запуске программы может быть велико
*/
    class Machine {

        CPU_State cpu;
        Memory mem;
        list<SEG_VADDR_DATA> memManage;

/*
    По хорошему здесь нужно сделать менеджер памяти, чтобы если у нас кусок памяти освобождался, на его место можно было что-то вставлять
*/

        void LoadDataFromFile(std::ifstream &file, size_t fileAlign, size_t sizeInFile, uint8_t flag){
            uint32_t freeVaddr = memManage.back().endVaddr;
            memManage.emplace_back(freeVaddr, freeVaddr + sizeInFile, flag);
            mem.WriteFromFile(freeVaddr, file, fileAlign, sizeInFile);
        }

    public:

/*
    Because of our JUMP vaddr must be multiply of the section size (256mb)
    Чтобы избавиться от необходимости 0x10000000 нужно сделать таблицу страниц итд итп
    Чтобы физический адрес начала кода был равен 0x10000000 (ну или 0х0) виртуального
*/

/*
                    Для более правдоподобной 
                        эмуляции системы тут \/ нужно вставить конец пространства адресов, зарезервированного ОС*/
        Machine(){ memManage.emplace_back(0, 0x10000000, PERMISSION_DENIED); };

        void run(string file_name);
    };

    } // namespace Interpretator
