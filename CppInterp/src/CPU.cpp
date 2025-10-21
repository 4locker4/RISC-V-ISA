#include "../inc/CPU.hpp"
#include "../inc/Decode.hpp"
#include "../inc/Execute.hpp"

using namespace Interpretator;

int main (int argc, char * argv[]) {
    if (argc < 2)
    {
        std::cout << "Fail. File needed. Try again..." << std::endl;
        return 0;
    }

    CPU cpu;
    Executor exec(cpu);

    if (argc == 2){
        cpu.LoadExecuteble(argv[1], 3 * 4);
        exec.run();
    } else if (argc == 4 && ! strcmp (argv[1], "-t")){  // Fibonacci test
        cpu.LoadExecuteble(argv[3], 0x10000000);
        cpu.WriteWord(0, std::stoi(argv[2]));
        cpu.WriteWord(4, 1);
        cpu.WriteWord(8, EXIT);
        cout << cpu.GetPC() << endl;
        exec.run();
    } else
        std::cout << "Unusual behavior" << std::endl;

    return 0;
}