#include <cinttypes>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <functional>
#include <iterator>
#include <cinttypes>

#include "VirtualMachine.hpp"
#include "Op.hpp"

std::string opName(OpCode op_code) {
    if (op_code == OP_HALT)
        return "halt";
    else if (op_code == OP_SET)
        return "set";
    else if (op_code == OP_PUSH)
        return "push";
    else if (op_code == OP_POP)
        return "pop";
    else if (op_code == OP_EQ)
        return "eq";
    else if (op_code == OP_GT)
        return "gt";
    else if (op_code == OP_JMP)
        return "jmp";
    else if (op_code == OP_JT)
        return "jt";
    else if (op_code == OP_JF)
        return "jf";
    else if (op_code == OP_ADD)
        return "add";
    else if (op_code == OP_MULT)
        return "mult";
    else if (op_code == OP_MOD)
        return "mod";
    else if (op_code == OP_AND)
        return "and";
    else if (op_code == OP_OR)
        return "or";
    else if (op_code == OP_NOT)
        return "not";
    else if (op_code == OP_RMEM)
        return "rmem";
    else if (op_code == OP_WMEM)
        return "wmem";
    else if (op_code == OP_CALL)
        return "call";
    else if (op_code == OP_RET)
        return "ret";
    else if (op_code == OP_OUT)
        return "out";
    else if (op_code == OP_IN)
        return "in";
    else if (op_code == OP_NOOP)
        return "noop";

    return "undefined";
}

int arg_count(OpCode op_code) {
    switch (op_code)
    {
    case OP_HALT:
    case OP_NOOP:
    case OP_RET:
    default:
        return 0;

    case OP_PUSH:
    case OP_POP:
    case OP_JMP:
    case OP_CALL:
    case OP_OUT:
    case OP_IN:
        return 1;

    case OP_SET:
    case OP_JT:
    case OP_JF:
    case OP_NOT:
    case OP_RMEM:
    case OP_WMEM:
        return 2;

    case OP_EQ:
    case OP_GT:
    case OP_ADD:
    case OP_MULT:
    case OP_MOD:
    case OP_AND:
    case OP_OR:
        return 3;
    }
}

bool is_op(const uint16_t& data) {
    return(data >= OP_HALT && data <= OP_NOOP);
};

int decode_op(std::ostream &os, std::vector<uint16_t> mem, int& i) {
    OpCode op_code = (OpCode) mem[i];
    os << std::setfill(' ') << std::setw(5) << opName(op_code);
    i++;

    int N = arg_count(op_code);
    for(int j = 0; j < N; j++) {
        os << std::setfill(' ')<< std::setw(7);
        uint16_t val = mem[i + j];
        if(val >= REG_LOW && val <= REG_HIGH) {
            os << "r" + std::to_string(val % REG_LOW);
        } else {
            os << val;
            if(op_code == OP_OUT && val >= 33 && val <= 126) {
                os << " " << (char)val;
            }
        }
    }
    os << std::setfill(' ') << std::setw(7) << "";

    return N - 1;
}