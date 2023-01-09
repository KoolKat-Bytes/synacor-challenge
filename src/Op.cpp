#include <cinttypes>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <functional>
#include <iterator>
#include <cinttypes>

#include "Op.hpp"
#include "VirtualMachine.hpp"

const std::string opName(OpCode op_code) {
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

std::ostream& operator<<(std::ostream& os, Op const& op)
{
    os << "[" << std::setfill(' ') << std::setw(5) << op.addr << "] ";
    os << " " << opName(op.op_code);

    for(auto it : op.args) {
        os << " " << std::setfill(' ') << std::setw(5);
        if(it >= REG_LOW && it <= REG_HIGH) {
            os << " r" << (it % REG_LOW);
        } else {
            os << it;
            if(op.op_code == OP_OUT && it >= 33 && it <= 126) {
                os << " " << std::setfill(' ') << std::setw(24);
                os << "; " << (char)it;
            }
        }
        os << " ";
    }

    os << "->[" << std::setfill(' ') << std::setw(5) << op.next_addr << "] ";
    
    return os;
}