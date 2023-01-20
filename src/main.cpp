#include <iostream>
#include <string>
#include <getopt.h>

#include "VirtualMachine.hpp"

const static struct option long_options[] = {
    {"file",    required_argument, 0, 'f'},
    {"play",    required_argument, 0, 'p'},
    {0,         0,                 0,  0 }
};

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <option(s)>\n"
              << "Options:\n"
              << "\t-f,--file FILE\tSpecify the binary file path to run\n"
              << "\t-p,--play FILE\tSpecify the file path to play\n"
              << "\t-h\t\tShow this help message\n"
              << std::endl;
}

int main(int argc, char** argv) {
    bool ret = true;
    VirtualMachine *vm = NULL;
    string fpath;
    string ppath;
    char opt;
    int opt_index = 0;

    while ((opt = getopt_long(argc, argv, "f:p:", long_options, &opt_index)) != -1)
    {
        switch (opt)
        {
        case 'f':
            fpath = string(optarg);
            break;
        case 'p':
            ppath = string(optarg);
            break;
        case '?':
            break;
        default:
            show_usage(argv[0]);
            break;
        }
    }

    if (!fpath.size()) {
        show_usage(argv[0]);
        goto out;
    }

    vm = VirtualMachine::get();

    if (ppath.size())
        vm->set_playfile(ppath);

    ret = vm->run(fpath);

    if (ret != true) {
        string dfname("./vm.dump");
        vm->dumpAsm(dfname);
        cerr << "Error: memory dumped to "
             << "'<CWD>/" << dfname << "'" << endl;
    }

out:
    if (vm)
        delete vm;

    return ret;
}