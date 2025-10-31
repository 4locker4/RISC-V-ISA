#include "../inc/Execute.hpp"
#include "../inc/Decode.hpp"
#include "../inc/Machine.hpp"

using namespace Interpretator;

int Executor::Execute(){
    Decoder decoder(cmd);

    while (true){
        auto opcode = decoder.DecodeData(mem.ReadWord(cpu.GetPC()));
        
        switch (opcode){

            case ADD:      ExecuteADD();      cpu.DumpRegisters(); break;
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

            case SYSCALL:{
                switch(ExecuteSYSCALL()){
                    case CONTINUE: break;
                    case ERROR:{
                        cout << "UNDEFINED ERROR IN SYSCALL!" << endl;
                        return 0;
                    }
                    case END_AND_PROCESS_DATA: return 0;
                    default:{
                        cout << "UNDEFINED SYSCALL RET" << endl;
                        return 0;
                    }
                }
            }
            default: break;
        }
        
        ChangePC(opcode);
    }

    cpu.DumpRegisters();
    
    return 0;
}