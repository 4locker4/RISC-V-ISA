#include "../inc/Decode.hpp"
#include "../inc/CPU.hpp"

using namespace Interpretator;

static const command_t OPCODE_MASK = 0b111111;

int Decoder::DecodeData (command_t cmd){

    uint32_t opcode = (cmd >> 26) & OPCODE_MASK;

    if (!opcode) {
        opcode = cmd & OPCODE_MASK;

        if (opcode == RBIT) DecodeR_TwoReg(cmd);
        else if (opcode == SYSCALL) ;
        else  DecodeR_ThreeReg(cmd);
    } else {
        switch(opcode) {
            case LD:
            case ST: {
                DecodeLD_ST(cmd);
                break;
            }
            case RORI:
            case USAT: {
                DecodeRORI_USAT(cmd);
                break;
            }
            case BEQ:
            case SLTI: {
                DecodeBEQ_SLTI (cmd);
                break;
            }
            case STP: {
                DecodeSTP (cmd);
                break;
            }
            case J: {
                DecodeJ (cmd);
                break;
            }
            default: {
                throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
                break;
            }
        }
    }

    return opcode;
}