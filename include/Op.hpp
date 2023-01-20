#ifndef _OP_HPP
#define _OP_HPP

#include <cinttypes>
#include <iostream>
#include <vector>

enum OpCode {
    OP_HALT = 0,
    OP_SET,
    OP_PUSH,
    OP_POP,
    OP_EQ,
    OP_GT,
    OP_JMP,
    OP_JT,
    OP_JF,
    OP_ADD,
    OP_MULT,
    OP_MOD,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_RMEM,
    OP_WMEM,
    OP_CALL,
    OP_RET,
    OP_OUT,
    OP_IN,
    OP_NOOP
};

std::string opName(OpCode op_code);
int arg_count(OpCode op_code);
bool is_op(const uint16_t& data);
int decode_op(std::ostream& os, std::vector<uint16_t> mem, int& i);

#endif /* _OP_HPP */