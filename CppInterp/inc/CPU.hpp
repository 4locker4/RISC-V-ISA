#pragma once

#include <cstdint>    // for uint32_t, uint8_t
#include <cstring>    // for size_t, memcpy
#include <fstream>    // for basic_ostream, operator<<, basic_ifstream, endl
#include <iomanip>    // for operator<<, setfill, setw
#include <iostream>   // for cout, cerr
#include <stdexcept>  // for out_of_range, runtime_error
#include <string>     // for char_traits, basic_string, string
#include <vector>     // for vector

using namespace std;

namespace Interpretator{

   enum OPCODES{
      ADD  = 0b011010,
      SUB  = 0b011111,
      MOVZ = 0b000100,
      SELC = 0b000011,
      RBIT = 0b011101,
      RORI = 0b001100,
      ST   = 0b100101,
      STP  = 0b111001,
      BEQ  = 0b010011,
      SLTI = 0b111011,
      USAT = 0b100010,
      J    = 0b010110,
      LD   = 0b100011,
      SYSCALL = 0b011001
   };

   typedef uint32_t command_t;
   typedef uint32_t reg_t;
   typedef uint32_t addr_t;
   typedef uint8_t mem_t;

   static const unsigned int EXIT = 93;
   static const unsigned int N_REGISTERS = 32;
   static const unsigned int N_MEM = 1024;

   typedef struct Command_st {
      reg_t rd : 5;
      reg_t rs : 5;
      reg_t rt : 5;
      uint32_t imm;
   } Command_st;

    class CPU{
        std::vector<mem_t> memory;
        reg_t registers[N_REGISTERS] = {0};
        reg_t PC = 0;
    
        public:
    
        reg_t GetReg(uint32_t regNum) { return registers[regNum]; }
        void SetReg(uint32_t regNum, reg_t value) { registers[regNum] = value; }
    
        reg_t GetPC() { return PC; }
        void IncrPC(uint32_t value) { PC += value; }
        void SetPC(reg_t value) { PC = value; }
    
/*
        Because of our JUMP vaddr must be multiply of the section size (256mb)
*/
        void LoadExecuteble (string file_name, size_t vaddr){
            std::ifstream file (file_name, std::ios::binary | std::ios::ate);
            if (!file.is_open ()){
            std::cerr << "Can not open the file!" << std::endl;
            }
        
            size_t fileSize = file.tellg ();
            file.seekg(std::ios::beg);
            memory.resize(fileSize + vaddr);
        
            file.read(reinterpret_cast<char *> (memory.data()) + vaddr, fileSize);
            file.close();
        
            PC = vaddr;
        }
    
        uint32_t ReadWord(reg_t index) const {
            if (index % sizeof(command_t) != 0) {
            cout << index << " + x7: " << registers[7] << endl;
            throw std::runtime_error("ReadWord: misaligned access");
            }
        
            if (index + sizeof(mem_t) > memory.size()) {
            throw std::out_of_range("ReadWord: address out of range");
            }
        
            uint32_t value;
            memcpy(&value, memory.data() + index, sizeof(uint32_t));
            return value;
        }
    
        void WriteWord(reg_t index, uint32_t value) {
            if (index % sizeof(command_t) != 0) {
            throw std::runtime_error("ReadWord: misaligned access");
            }
        
            if (index + sizeof(mem_t) > memory.size()) {
            throw std::out_of_range("ReadWord: address out of range");
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
    
        void dump_registers() const {
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
} // namespace Interpretator
