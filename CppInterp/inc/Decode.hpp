#pragma once

#include <stdint.h>   // for int16_t, uint32_t, int8_t
#include <bitset>     // for operator<<, bitset
#include <iomanip>    // for operator<<, setfill, setw
#include <iostream>   // for basic_ostream, operator<<, cout, endl, basic_ios
#include <stdexcept>  // for runtime_error
#include <string>     // for char_traits, allocator, basic_string, operator==
#include "./CPU.hpp"  // for command_t, reg_t, Command_st

using namespace std;

namespace Interpretator{
    constexpr uint32_t SignExtend(uint32_t value, int srcNumSz) {
        uint32_t mask = (1U << srcNumSz) - 1;
        value &= mask;
        uint32_t sign_bit = 1U << (srcNumSz - 1);
        
        if (value & sign_bit)
            value |= ~mask;
        
        return value;
    }
    
    class Decoder{
        Command_st &cmd;

        void dumPCommand(command_t cmd, const string& type, const string& name) {
            cout << "----------------------------------------" << endl;
            cout << type << " Instruction:" << endl;
        
            if (type == "R-Type") {
                cout << "funct:     " << bitset<6>(cmd & 0x3F) << " (" << ((cmd) & 0x3F) << ")" << endl;
                cout << "rs:        " << bitset<5>((cmd >> 21) & 0x1F) << " (" << ((cmd >> 21) & 0x1F) << ")" << endl;
                cout << "rt:        " << bitset<5>((cmd >> 16) & 0x1F) << " (" << ((cmd >> 16) & 0x1F) << ")" << endl;
                cout << "rd:        " << bitset<5>((cmd >> 11) & 0x1F) << " (" << ((cmd >> 11) & 0x1F) << ")" << endl;
            } else if (type == "I-Type") {
               cout << "funct:     " << bitset<6>((cmd >> 26) & 0x3F) << " (" << ((cmd >> 26) & 0x3F) << ")" << endl;
               cout << "rs:        " << bitset<5>((cmd >> 21) & 0x1F) << " (" << ((cmd >> 21) & 0x1F) << ")" << endl;
               cout << "rt:        " << bitset<5>((cmd >> 16) & 0x1F) << " (" << ((cmd >> 16) & 0x1F) << ")" << endl;
               cout << "immediate: " << bitset<16>(cmd & 0xFFFF) << " (" << static_cast<int16_t>(cmd & 0xFFFF) << ")" << endl;
            } else if (type == "J-Type") {
               cout << "funct:     " << bitset<6>((cmd >> 26) & 0x3F) << " (" << ((cmd >> 26) & 0x3F) << ")" << endl;
               cout << "address:   " << bitset<26>(cmd & 0x3FFFFFF) << " (" << (cmd & 0x3FFFFFF) << ")" << endl;
            }
         
            cout << "bytes:     ";
            for (int i = 3; i >= 0; --i) {
                cout << bitset<8>((cmd >> (i * 8)) & 0xFF);
                if (i > 0) cout << " ";
            }
            cout << endl;

            cout << "hex:       0x" << hex << setw(8) << setfill('0') << cmd << dec << endl;
            cout << "----------------------------------------" << endl;
        }

        inline command_t GetOPCode(command_t &cmd)    { return cmd & 0b111111; }  
        inline reg_t     GetRegOffs21(command_t &cmd) { return (cmd >> 21) & 0b11111; }
        inline reg_t     GetRegOffs16(command_t &cmd) { return (cmd >> 16) & 0b11111; }
        inline reg_t     GetRegOffs11(command_t &cmd) { return (cmd >> 11) & 0b11111; }
        inline int16_t   GetImm16(command_t &cmd)     { return static_cast<int16_t>(cmd & 0xFFFF); }
        inline int16_t   GetImm11(command_t &cmd)     { return static_cast<int16_t>(cmd & 0x7FFF); }
        inline int8_t    GetImm8(command_t &cmd)      { return (cmd >> 11) & 0b11111; }
        inline uint32_t  GetJAddr(command_t &cmd)     { return cmd & 0x3FFFFFF; }

        void DecodeR_ThreeReg (command_t &cmd_bin) {
            dumPCommand(cmd_bin, "R-Type", "\"R-Type\"");
        
            cmd.rs = GetRegOffs21(cmd_bin);
            cmd.rt = GetRegOffs16(cmd_bin);
            cmd.rd = GetRegOffs11(cmd_bin);
        }
    
        void DecodeR_TwoReg (command_t &cmd_bin) {
           dumPCommand(cmd_bin, "R-Type", "\"R-Type\"");

           cmd.rd = GetRegOffs21(cmd_bin);
           cmd.rs = GetRegOffs16(cmd_bin);
        }

       void DecodeSTP (command_t &cmd_bin) {
            dumPCommand(cmd_bin, "I-Type", "\"I-Type\"");
    
            cmd.rd = GetRegOffs21(cmd_bin);
            cmd.rs = GetRegOffs16(cmd_bin);
            cmd.rt = GetRegOffs11(cmd_bin);
            cmd.imm = SignExtend(cmd_bin & 0x7FFF, 11);
        }

        void DecodeBEQ_SLTI(command_t &cmd_bin) {
            dumPCommand(cmd_bin, "I-Type", "\"I-Type\"");
    
            cmd.rs = GetRegOffs21(cmd_bin);
            cmd.rt = GetRegOffs16(cmd_bin);
            cmd.imm = SignExtend((cmd_bin & 0xFFFF), 16);
        }

        void DecodeRORI_USAT(command_t &cmd_bin) {
            dumPCommand(cmd_bin, "I-Type", "\"I-Type\"");
    
            cmd.rt = GetRegOffs21(cmd_bin);
            cmd.rs = GetRegOffs16(cmd_bin);
            cmd.imm = (cmd_bin >> 11) & 0b11111;
        }

        void DecodeLD_ST(command_t &cmd_bin) {
            dumPCommand(cmd_bin, "I-Type", "\"I-Type\"");
    
            cmd.rd = GetRegOffs21(cmd_bin);
            cmd.rt = GetRegOffs16(cmd_bin);
            cmd.imm = SignExtend(cmd_bin & 0xFFFF, 16);
    
            if ((cmd.imm & 0b11) != 0) {
            throw runtime_error("LD: misaligned access (offset not word-aligned)");
            }
        }
    
        void DecodeJ(command_t &cmd_bin) {
            dumPCommand(cmd_bin, "J-Type", "\"J-Type\"");

            cmd.imm = cmd_bin & 0x3FFFFFF;
        }

        void DecodeSYSCALL () {}

        public:

        int DecodeData(command_t cmd);

        Decoder(Command_st &command) : cmd (command) {}
        Decoder() = delete;
   };

}