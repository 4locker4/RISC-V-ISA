#include "../inc/Execute.hpp"
#include "../inc/Decode.hpp"
#include "../inc/CPU.hpp"

using namespace Interpretator;

int Executor::run(){
    Decoder decoder(cmd);

    while (true){
        auto opcode = decoder.DecodeData(cpu.ReadWord(cpu.GetPC()));
        
        switch (opcode){
            case ADD:      {ExecuteADD();   cpu.dump_registers();   break;}
            case SUB:      ExecuteSUB();      break;
            case MOVZ:     ExecuteMOVZ();     break;
            case SELC:     ExecuteSELC();     break;
            case RBIT:     ExecuteRBIT();     break;
            case ST:       ExecuteST();       break;
            case STP:      ExecuteSTP();      break;
            case BEQ:      ExecuteBEQ();      break;
            case SLTI:     ExecuteSLTI();     break;
            case USAT:     ExecuteUSAT();     break;
            case RORI:     ExecuteRORI();     break;
            case J:        ExecuteJ();        break;
            case LD:       ExecuteLD();       break;
            case SYSCALL:  ExecuteSYSCALL();  break;
            default: break;
        }

        if (opcode == SYSCALL && cpu.GetReg(7) == EXIT) {
            cpu.dump_registers();
            return 0;
        }

        cpu.IncrPC(sizeof(command_t));
    }
    cpu.dump_registers();
    return 0;
}