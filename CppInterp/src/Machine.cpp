#include "../inc/Machine.hpp"
#include "../inc/Decode.hpp"
#include "../inc/Execute.hpp"

using namespace Interpretator;

int main (int argc, char * argv[]) {
    if (argc < 2)
    {
        std::cout << "Fail. File needed. Try again..." << std::endl;
        return 0;
    }

    Machine mc;

    if (argc == 2) mc.run(argv[1]);
    else std::cout << "Unusual behavior" << std::endl;

    return 0;
}

void Machine::run(string file_name){
    std::ifstream file (file_name, std::ios::binary | std::ios::ate);
    if (!file.is_open ())
        std::cerr << "Can not open the file!" << std::endl;

    size_t fileSize = file.tellg();

    LoadDataFromFile(file, std::ios::beg, fileSize, EXECUTABLE);
    cpu.SetPC(memManage.back().startVaddr);

    file.close();

    Executor Exec(cpu, mem);
    Exec.Execute();
}