#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <iostream>

#include <iomanip>
#include <bitset>
#define NDEBUG
#ifdef NDEBUG
#define DUMP_COMMAND(cmd, type) dumpCommand(cmd, type, #type)
#else
#define DUMP_COMMAND(cmd, type)
#endif

typedef std::int32_t command_t;
typedef std::int32_t reg_t;
typedef int32_t addr_t;

static const reg_t EXIT = 93;
static const reg_t N_REGISTERS = 32;
static const reg_t N_MEM = 1024;

/* Суть
   
   Мы закидываем в интерпретатор команды в бинарном виде
   Декодируем их
   Выполняем действия с регистрами (не на прямую через алу, а оператором "+" в с++ коде), то есть по сути отличие от компилируемого языка в том, что 
   данная ISA транслируется в с++ код и выполняется им уже
   короче мой asm транслируется в с++, который потом транслируется в другой asm и исполняется. Вот мораль
   А регистры это единственные доступные переменные (пока что) 
*/

/* TD
   Раскидать каждую команду по вектору структур
   Создать класс, в котором будет память и храниться регистры будут
   +РС тоже надо понимать
   Пишем обработчик команд
   Исполняем по одной 
*/

/* Что должен включать класс? 
   Public:
      Ctorы, куда вставляют имя бин файла
   
   !! Далее везде должна быть проверка на валидность !!

      Испольнить [функции-члены: полностью или следующую команду с выводом в консоль].
      Выдать ассемблер всех команд [функции-члены: поток или файл].
      Выдать значение регистров [функции-члены: поток или файл]
      Выдать значение памяти [функции-члены: поток или файл]
   Privat:
      Все регистры [списком, потому что ничего добавлять не будем], инициализировать нулями. Сделать защиту РС и х0  
      Память [списком, потому что ничего добавлять не будем], инициализировать нулями
      Декодер {
         сделать через шитые свичи
      }
*/

class Interpretator{
   std::string file_n;
   
   reg_t registers[N_REGISTERS] = {0};
   reg_t &pc = registers[31];
   std::int32_t   memory[N_MEM]  = {0};
   std::unordered_map<command_t, std::function<void(command_t &cmd)>> command_table {
      {0b011010, [this](command_t &cmd) { ExecuteADD  (cmd); }},
      {0b011111, [this](command_t &cmd) { ExecuteSUB  (cmd); }},
      {0b000100, [this](command_t &cmd) { ExecuteMOVZ (cmd); }},
      {0b000011, [this](command_t &cmd) { ExecuteSELC (cmd); }},
      {0b011101, [this](command_t &cmd) { ExecuteRBIT (cmd); }},
      {0b100101, [this](command_t &cmd) { ExecuteST   (cmd); }},  
      {0b111001, [this](command_t &cmd) { ExecuteSTP  (cmd); }}, 
      {0b010011, [this](command_t &cmd) { ExecuteBEQ  (cmd); }},
      {0b111011, [this](command_t &cmd) { ExecuteSLTI (cmd); }},
      {0b100010, [this](command_t &cmd) { ExecuteUSAT (cmd); }},
      {0b001100, [this](command_t &cmd) { ExecuteRORI (cmd); }},
      {0b010110, [this](command_t &cmd) { ExecuteJ    (cmd); }},
      {0b100011, [this](command_t &cmd) { ExecuteLD   (cmd); }},
      {0b011001, [this](command_t &cmd) { ExecuteSYSCALL (cmd); }}
   };

   void dumpCommand(command_t cmd, const std::string& type, const std::string& name) {
      std::cout << "----------------------------------------" << std::endl;
      std::cout << type << " Instruction:" << std::endl;
      
      if (type == "R-Type") {
         std::cout << "funct:     " << std::bitset<6>(cmd & 0x3F) << " (" << ((cmd) & 0x3F) << ")" << std::endl;
         std::cout << "rs:        " << std::bitset<5>((cmd >> 21) & 0x1F) << " (" << ((cmd >> 21) & 0x1F) << ")" << std::endl;
         std::cout << "rt:        " << std::bitset<5>((cmd >> 16) & 0x1F) << " (" << ((cmd >> 16) & 0x1F) << ")" << std::endl;
         std::cout << "rd:        " << std::bitset<5>((cmd >> 11) & 0x1F) << " (" << ((cmd >> 11) & 0x1F) << ")" << std::endl;
      } else if (type == "I-Type") {
         std::cout << "funct:     " << std::bitset<6>((cmd >> 26) & 0x3F) << " (" << ((cmd >> 26) & 0x3F) << ")" << std::endl;
         std::cout << "rs:        " << std::bitset<5>((cmd >> 21) & 0x1F) << " (" << ((cmd >> 21) & 0x1F) << ")" << std::endl;
         std::cout << "rt:        " << std::bitset<5>((cmd >> 16) & 0x1F) << " (" << ((cmd >> 16) & 0x1F) << ")" << std::endl;
         std::cout << "immediate: " << std::bitset<16>(cmd & 0xFFFF) << " (" << static_cast<int16_t>(cmd & 0xFFFF) << ")" << std::endl;
      } else if (type == "J-Type") {
         std::cout << "funct:     " << std::bitset<6>((cmd >> 26) & 0x3F) << " (" << ((cmd >> 26) & 0x3F) << ")" << std::endl;
         std::cout << "address:   " << std::bitset<26>(cmd & 0x3FFFFFF) << " (" << (cmd & 0x3FFFFFF) << ")" << std::endl;
      }
      
      std::cout << "bytes:     ";
      for (int i = 3; i >= 0; --i) {
          std::cout << std::bitset<8>((cmd >> (i * 8)) & 0xFF);
          if (i > 0) std::cout << " ";
      }
      std::cout << std::endl;
      
      std::cout << "hex:       0x" << std::hex << std::setw(8) << std::setfill('0') << cmd << std::dec << std::endl;
      std::cout << "----------------------------------------" << std::endl;
  }

   void ExecuteADD (command_t &cmd) {
      DUMP_COMMAND(cmd, "R-Type");

      reg_t rs = (cmd >> 21) & 0b11111;
      reg_t rt = (cmd >> 16) & 0b11111;
      reg_t rd = (cmd >> 11) & 0b11111;

      registers[rd] = registers[rs] + registers[rt];

      pc += 4;

      return;
   }

   void ExecuteSUB (command_t &cmd) {
      DUMP_COMMAND(cmd, "R-Type");

      reg_t rs = (cmd >> 21) & 0b11111;
      reg_t rt = (cmd >> 16) & 0b11111;
      reg_t rd = (cmd >> 11) & 0b11111;

      registers[rd] = registers[rs] - registers[rt];

      pc += 4;

      return;
   }

   void ExecuteMOVZ (command_t &cmd) {
      DUMP_COMMAND(cmd, "R-Type");

      reg_t rs = (cmd >> 21) & 0b11111;
      reg_t rt = (cmd >> 16) & 0b11111;
      reg_t rd = (cmd >> 11) & 0b11111;

      if (registers[rt] == 0)
         registers[rd] = registers[rs];

      pc += 4;

      return;
   }

   void ExecuteSELC (command_t &cmd) {
      DUMP_COMMAND(cmd, "R-Type");

      reg_t rs1 = (cmd >> 21) & 0b11111;
      reg_t rs2 = (cmd >> 16) & 0b11111;
      reg_t rd = (cmd >> 11) & 0b11111;

      registers[rd] = (registers[rs1] > registers[rs2]) ? registers[rs1] : registers[rs2];

      pc += 4;

      return;
   }

   void ExecuteRBIT (command_t &cmd) {
      DUMP_COMMAND(cmd, "R-Type");

      reg_t rd = (cmd >> 21) & 0b11111;
      reg_t rs = (cmd >> 16) & 0b11111;

      reg_t x = registers[rs];
      x = ((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1);
      x = ((x & 0xCCCCCCCC) >> 2) | ((x & 0x33333333) << 2);
      x = ((x & 0xF0F0F0F0) >> 4) | ((x & 0x0F0F0F0F) << 4);
      x = ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
      registers[rd] = (x >> 16) | (x << 16);

      pc += 4;

      return;
   }

   void ExecuteST (command_t &cmd) {
      DUMP_COMMAND(cmd, "I-Type");

      reg_t base = (cmd >> 21) & 0b11111;
      reg_t rt = (cmd >> 16) & 0b11111;
      int offs_imm16 = static_cast<int16_t>(cmd & 0xFFFF);

      memory[(registers[base] + offs_imm16) / 4] = registers[rt];

      pc += 4;

      return;
   }

   void ExecuteSTP (command_t &cmd) {
      DUMP_COMMAND(cmd, "I-Type");

      reg_t base = (cmd >> 21) & 0b11111;
      reg_t rt1 = (cmd >> 16) & 0b11111;
      reg_t rt2 = (cmd >> 11) & 0b11111;
      addr_t offs_imm11 = static_cast<int16_t>(cmd & 0x7FF);

      addr_t addr = (registers[base] + offs_imm11) / 4;
      memory[addr] = rt1;
      memory[addr + 1] = rt2;

      pc += 4;

      return;
   }

   void ExecuteSLTI(command_t &cmd) {
      DUMP_COMMAND(cmd, "I-Type");

      reg_t rs = (cmd >> 21) & 0b11111;
      reg_t rt = (cmd >> 16) & 0b11111;
      addr_t imm = static_cast<int16_t>(cmd & 0xFFFF); // 16-битное знаковое
  
      registers[rt] = (registers[rs] < imm) ? 1 : 0;

      pc += 4;

      return;
  }

  void ExecuteBEQ(command_t &cmd) {
      DUMP_COMMAND(cmd, "I-Type");

      reg_t rs = (cmd >> 21) & 0b11111;
      reg_t rt = (cmd >> 16) & 0b11111;
      addr_t offset = static_cast<int16_t>(cmd & 0xFFFF);

      addr_t target = offset << 2;

      if (registers[rs] == registers[rt])
         pc += target;
      else
         pc += 4;

      return;
   }

   void ExecuteUSAT(command_t &cmd) {
      DUMP_COMMAND(cmd, "I-Type");

      reg_t rd = (cmd >> 21) & 0b11111;
      reg_t rs = (cmd >> 16) & 0b11111;
      uint8_t imm5 = (cmd >> 11) & 0b11111;

      if (imm5 == 0) 
           registers[rd] = 0;
      else {
           uint32_t max_val = (1U << imm5) - 1;
           uint32_t val = registers[rs];
           registers[rd] = (val > max_val) ? max_val : val;
      }

      pc += 4;

      return;
   }

   void ExecuteRORI(command_t &cmd) {
      DUMP_COMMAND(cmd, "I-Type");

      reg_t rd = (cmd >> 21) & 0b11111;
      reg_t rs = (cmd >> 16) & 0b11111;
      uint8_t imm5 = (cmd >> 11) & 0b11111;
  
      uint32_t val = registers[rs];
  
      if (imm5)
         registers[rd] = (val >> imm5) | (val << (32 - imm5));
      else 
         registers[rd] = val;
  
      pc += 4;

      return;
  }

   void ExecuteJ(command_t &cmd) {
      DUMP_COMMAND(cmd, "J-Type");

      addr_t index = cmd & 0x3FFFFFF;
  
      // Составляем новый PC: старшие 4 бита текущего PC + index + 0b00
      pc = (pc & 0xF0000000) | (index << 2);

      return;
   }

  void ExecuteLD(command_t &cmd) {
      DUMP_COMMAND(cmd, "I-Type");

      reg_t base = (cmd >> 21) & 0b11111;
      reg_t rt   = (cmd >> 16) & 0b11111;
      addr_t offset = static_cast<int16_t>(cmd & 0xFFFF);

      if ((offset & 0b11) != 0) {
          throw std::runtime_error("LD: misaligned access (offset not word-aligned)");
      }

      addr_t addr = (registers[base] + offset) / 4;
      registers[rt] = memory[addr];

      pc += 4;

      return;
   }

   void ExecuteSYSCALL (command_t &cmd) {
      switch (registers[7])
      {
         case EXIT:
            std::exit(0);
         default:
            std::runtime_error ("Syscall: enable syscall");
      }

      pc += 4;
   }

   std::vector<command_t> commands = {0};
   int Decoder (command_t &command_data);

   public:

   Interpretator(const std::string& file_name) : file_n (file_name) {}

   int Run ();

   int RegistersOut ();
   int MemOut ();
   int FibanacciTest (int n_nums);
};
