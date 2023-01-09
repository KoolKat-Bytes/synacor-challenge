
#include <iostream>
#include <string>

#include "VirtualMachine.hpp"

int main(int argc, char** argv) {
    bool ret = true;
    VirtualMachine *vm = NULL;
    string fpath;

    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <filename.bin>" << std::endl;
        ret = false;
        goto out;
    }

    fpath = string((argv[1]));

    vm = new VirtualMachine();

    ret = vm->run(fpath);

out:
    if (vm)
        delete vm;

    return ret;
}