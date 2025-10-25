#pragma once

#include <stdint.h>   // for uint32_t, int32_t
#include <iostream>   // for char_traits, basic_ostream, operator<<, basic_ios
#include <stdexcept>  // for runtime_error
#include "./CPU.hpp"  // for CPU, reg_t, addr_t, command_t, Command_st

using namespace std;

namespace Interpretator{
   class Executor{
      Command_st cmd = {};

      CPU &cpu;

      void ExecuteADD () {
         reg_t value = cpu.GetReg(cmd.rs) + cpu.GetReg(cmd.rt);
         cpu.SetReg(cmd.rd, value);
      }

      void ExecuteSUB() {
         reg_t rs = cpu.GetReg(cmd.rs);
         reg_t rt = cpu.GetReg(cmd.rt);

         cpu.SetReg(cmd.rd, rs - rt);
      }

      void ExecuteMOVZ() {
         if (cpu.GetReg(cmd.rt) == 0) {
            reg_t rs = cpu.GetReg(cmd.rs);
            cpu.SetReg(cmd.rd, rs);
         }
      }

      void ExecuteSELC() {
         reg_t rt = cpu.GetReg(cmd.rt);
         reg_t rd = cpu.GetReg(cmd.rd);
         reg_t result = (rt > rd) ? rt : rd;

         cpu.SetReg(cmd.rs, result);
      }

      void ExecuteRBIT() {
         reg_t x = cpu.GetReg(cmd.rs);
      
         x = ((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1);
         x = ((x & 0xCCCCCCCC) >> 2) | ((x & 0x33333333) << 2);
         x = ((x & 0xF0F0F0F0) >> 4) | ((x & 0x0F0F0F0F) << 4);
         x = ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
         reg_t result = (x >> 16) | (x << 16);
      
         cpu.SetReg(cmd.rd, result);
      }

      void ExecuteST() {
         reg_t base = cpu.GetReg(cmd.rd);
         reg_t value = cpu.GetReg(cmd.rt);
         addr_t addr = base + cmd.imm;

         cpu.WriteWord(addr, value);
      }

      void ExecuteSTP() {
         reg_t base = cpu.GetReg(cmd.rd);
         addr_t addr = base + cmd.imm;

         cpu.WriteWord(addr, cpu.GetReg(cmd.rs));
         cpu.WriteWord(addr + sizeof(reg_t), cpu.GetReg(cmd.rt));
      }

      void ExecuteSLTI() {
         reg_t rs = cpu.GetReg(cmd.rs);
// cmd.imm уже знаково расширен в Decoder
         reg_t result = (rs < cmd.imm) ? 1 : 0;

         cpu.SetReg(cmd.rt, result);
      }

      void ExecuteBEQ() {
         reg_t rs = cpu.GetReg(cmd.rs);
         reg_t rt = cpu.GetReg(cmd.rt);

         if (rs == rt) {
// cmd.imm уже знаково расширен в Decoder
            cpu.SetPC(cpu.GetPC() + cmd.imm);
         }
      }

      void ExecuteUSAT() {
         if (cmd.imm == 0) {
            cpu.SetReg(cmd.rt, 0);
         } else {
            uint32_t max_val = (1U << cmd.imm) - 1;
            uint32_t val = cpu.GetReg(cmd.rs);
            cpu.SetReg(cmd.rt, (val > max_val) ? max_val : val);
         }
      }

      void ExecuteRORI() {
         reg_t val = cpu.GetReg(cmd.rs);
         reg_t result;

         if (cmd.imm) {
            result = (val >> cmd.imm) | (val << (32 - cmd.imm));
         } else {
            result = val;
         }

         cpu.SetReg(cmd.rt, result);
      }

      void ExecuteJ() {
         uint32_t next_pc = cpu.GetPC() + sizeof(command_t);   // MIPS arch
         reg_t target = (next_pc & 0xF0000000) | (cmd.imm << 2);
         cpu.SetPC(target - sizeof(command_t));    // Because of we add sizeof(command_t) after execute
      }

      void ExecuteLD() {
         if ((cmd.imm & 0b11) != 0) {
            throw std::runtime_error("LD: misaligned access (offset not word-aligned)");
         }
         reg_t base = cpu.GetReg(cmd.rd);
         addr_t addr = base + cmd.imm;
         cout << cmd.rt << " - base; " << cmd.imm << " - imm;" << endl;

         reg_t value = cpu.ReadWord(addr);

         cpu.SetReg(cmd.rt, value);
      }

      void ExecuteSYSCALL() {}

     public:

     int run();
     Executor(CPU &cpuCl) : cpu{cpuCl} {}
     Executor() = delete;
   };
} // namespace Interpretator