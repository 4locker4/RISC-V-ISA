#include "../inc/Interpr.hpp"
#include <fstream>
#include <stddef.h> 
#include <utility>

static const command_t OPCODE_MASK = 0b111111;
int main (int argc, char * argv[])
{
    if (argc < 2)
    {
        std::cout << "Fail. File needed. Try again..." << std::endl;
        return 0;
    }

    Interpretator prog (argv[1]);
    prog.Run ();

    return 0;
}

int Interpretator::Run ()
{
    std::ifstream file (file_n, std::ios::binary | std::ios::ate);
    if (!file.is_open ())
    {
        std::cerr << "Can not open the file!" << std::endl;
        return -1;
    }

    size_t file_size = file.tellg ();
    commands.resize(file_size / sizeof (command_t));
    file.seekg(std::ios::beg);

    file.read(reinterpret_cast<char *> (commands.data()), file_size);
    file.close();

    for (auto cmd : commands)
    {
        std::cout << std::bitset<32>(cmd) << std::endl;
        Decoder (cmd);
    }

    return 0;
}

int Interpretator::Decoder (command_t &command_data)
{
    uint32_t opcode = (command_data >> 26) & OPCODE_MASK;

    if (!opcode) {
        std::cout << "бебебе" << std::endl;
        opcode = command_data & OPCODE_MASK;
        std::cout << std::bitset<6>(opcode) << std::endl;
    }
    std::cout << std::bitset<32>(command_data) << std::endl;

    auto do_cmd = command_table.find (opcode);
    if (do_cmd != command_table.end ())
        do_cmd->second (command_data);
    else
        throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));

    return 0;
}