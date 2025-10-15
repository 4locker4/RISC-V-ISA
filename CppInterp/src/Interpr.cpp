#include "../inc/Interpr.hpp"
#include <fstream>
#include <stddef.h> 
#include <utility>
#include <cstring>

static const command_t OPCODE_MASK = 0b111111;
int main (int argc, char * argv[])
{
    if (argc < 2)
    {
        std::cout << "Fail. File needed. Try again..." << std::endl;
        return 0;
    }

    Interpretator prog (argv[1]);

    if (! strcmp (argv[2], "-t"))
        prog.FibanacciTest (std::stoi (argv[3]));
    else
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

    for (int i = 0; commands.size () > i; )
    {
        Decoder (commands[i]);

        i = pc / 4;

        std::cout << i << std::endl;
    }

    return 0;
}

int Interpretator::Decoder (command_t &command_data)
{
    uint32_t opcode = (command_data >> 26) & OPCODE_MASK;

    if (!opcode) {
        std::cout << "бебебе" << std::endl;
        opcode = command_data & OPCODE_MASK;
    }

    auto do_cmd = command_table.find (opcode);
    if (do_cmd != command_table.end ())
        do_cmd->second (command_data);
    else
        throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));

    return 0;
}

int Interpretator::RegistersOut ()
{
    for (int i = 0; N_REGISTERS > i; i++)
        std::cout << "R" << i << ": " << registers[i] << std::endl;

    return 0;
}

int Interpretator::MemOut ()
{
    for (int i = 0; N_MEM > i; i += 8)
    {
        for (int j = 0; 8 > j; j++)
            std::cout << memory[i + j] << " ";
        std::cout << std::endl;
    }

    return 0;
}

int Interpretator::FibanacciTest (int n_nums){
    memory[0] = n_nums;
    memory[1] = 1;
    memory[2] = EXIT;

    MemOut ();
    RegistersOut ();

    Run ();

    MemOut ();
    RegistersOut ();

    return 0;
}