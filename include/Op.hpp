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

inline bool is_op(const uint16_t& data) {
    return(data >= OP_HALT && data <= OP_NOOP);
};

class Op {
public:
    friend std::ostream& operator<<(std::ostream& os, Op const& op);

    Op(const uint16_t& _addr, const OpCode& _op_code, const uint16_t& _next_addr,
        const std::vector<uint16_t>& _args) :
        addr(_addr), next_addr(_next_addr), op_code(_op_code), args(_args) {};

private:
    const uint16_t addr;
    const OpCode op_code;
    std::vector<uint16_t> args;

    const uint16_t next_addr;
};

#endif /* _OP_HPP */

