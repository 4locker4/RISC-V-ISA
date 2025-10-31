#pragma once

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

   enum REGISTERS{
        X0, X1, X2,
        X3, X4, X5,
        X6, X7, X8,
        X9, X10, X11,
        X12, X13, X14,
        X15, X16, X17,
        X18, X19, X20,
        X21, X22, X23,
        X24, X25, X26, 
        X27, X28, X29,
        X30, X31
   };

   enum SYSCALL_NUMS{
        TERMINAL_INPUT = 0,
        EXIT = 93
   };

   enum SYSCALL_RETS{
        CONTINUE,
        ERROR,
        END_AND_PROCESS_DATA
   };

   enum MEM_FLAGS{
        PERMISSION_DENIED,
        EXECUTABLE = 1 << 0,
        READ_ONLY = 1 << 1,
   };
} // Interpretator namespace