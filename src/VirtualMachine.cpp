#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <iterator>
#include <cinttypes>

#include "Op.hpp"
#include "VirtualMachine.hpp"

VirtualMachine* VirtualMachine::vm = VirtualMachine::get();

VirtualMachine* VirtualMachine::get() {
    if (!vm)
        vm = new VirtualMachine();

    return vm;
}

VirtualMachine::VirtualMachine() {
    this->state = VM_INIT;
    this->pos = 0;
    this->_registers = vector<uint16_t>(REG_COUNT);
}

void VirtualMachine::extractText(std::string& d_path) {
    ofstream os;

    os.open(d_path);

    if(!os.is_open()) {
        std::cerr << "failed to open: '" << d_path << "'" << std::endl;
        return;
    }

    /* 33 -> 126 printable chars and 10 is new line */
    for(auto const& it : this->_memory) {
        if(it >= 32 && it <= 126 || it == 10)
            os << (char)it;
    }
    os.close();
}

void VirtualMachine::dumpAsm(std::string& d_path) {
    ofstream os;

    os.open(d_path);

    if(!os.is_open()) {
        std::cerr << "failed to open: '" << d_path << "'" << std::endl;
        return;
    }

    /* print past operations */
    for(auto const &it : this->ops)
        os << *it.second << std::endl;

    /* print what is left in memory */
    auto last = this->ops.crbegin();
    if (last == this->ops.rend())
        return;

    int i = last->first;
    size_t msize = this->_memory.size();

    for(/* nothing */; i < msize; i++) {
        os << "[" << std::setfill(' ') << std::setw(5) << i << "] ";
        os << " mem " << this->_memory[i];

        /* 33 -> 126 printable chars and 10 is new line */
        if(this->_memory[i] >= 33 && this->_memory[i] <= 126) {
            os << " " << std::setfill(' ') << std::setw(24);
            os << "; " << (char)this->_memory[i];
        }
        os << std::endl;
    }

    os.close();
}

bool VirtualMachine::loadFile(std::string& s_path) {
    ifstream is;

    is.open(s_path, std::ios::binary);

    if(!is.is_open()) {
        std::cerr << "failed to open: '" << s_path << "'" << std::endl;
        return false;
    }

    std::streampos fsize;

    is.seekg(0, std::ios::end);
    fsize = is.tellg();
    is.seekg(0, std::ios::beg);

    // NOTE: assuming file size is no more than MEMORY SIZE
    // TODO: read chunk by chunk otherwise
    this->_memory.resize(fsize / sizeof(uint16_t));
    is.read((char*)(this->_memory.data()), fsize);

    is.close();

    return true;
}

bool VirtualMachine::run(std::string& s_path) {

    if(!this->loadFile(s_path)) {
        std::cerr << "Failed to load file to the virtual machine" << std::endl;
        return false;
    }

    /* start execution */
    this->state = VM_RUN;

    const size_t msize = this->_memory.size();
    uint16_t &i = this->pos;

    while(this->state == VM_RUN) {

        uint16_t op_code = this->_memory[i];
        std::vector<uint16_t> args;
        const uint16_t curr_addr = i;

        switch(op_code)
        {
        case OP_HALT:
            this->_halt();
            no_operand_next(i);
            break;

        case OP_SET:
            this->_set(this->_memory[i + 1], this->_memory[i + 2]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            two_operand_next(i);
            break;
        case OP_PUSH:
            this->_push(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 1]);
            one_operand_next(i);
            break;
        case OP_POP:
            this->_pop(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 1]);
            one_operand_next(i);
            break;
        case OP_EQ:
            this->_eq(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            args.push_back(this->_memory[i + 3]);
            three_operand_next(i);
            break;
        case OP_GT:
            this->_gt(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            args.push_back(this->_memory[i + 3]);
            three_operand_next(i);
            break;
        case OP_JMP: /* have control over position */
            this->_jmp(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 1]);
            break;
        case OP_JT: /* have control over position */
            this->_jt(this->_memory[i + 1], this->_memory[i + 2]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            break;
        case OP_JF: /* have control over position */
            this->_jf(this->_memory[i + 1], this->_memory[i + 2]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            break;
        case OP_ADD:
            this->_add(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            args.push_back(this->_memory[i + 3]);
            three_operand_next(i);
            break;
        case OP_MULT:
            this->_mult(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            args.push_back(this->_memory[i + 3]);
            three_operand_next(i);
            break;
        case OP_MOD:
            this->_mod(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            args.push_back(this->_memory[i + 3]);
            three_operand_next(i);
            break;
        case OP_AND:
            this->_and(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            args.push_back(this->_memory[i + 3]);
            three_operand_next(i);
            break;
        case OP_OR:
            this->_or(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            args.push_back(this->_memory[i + 3]);
            three_operand_next(i);
            break;
        case OP_NOT:
            this->_not(this->_memory[i + 1], this->_memory[i + 2]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            two_operand_next(i);
            break;
        case OP_RMEM:
            this->_rmem(this->_memory[i + 1], this->_memory[i + 2]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            two_operand_next(i);
            break;
        case OP_WMEM:
            this->_wmem(this->_memory[i + 1], this->_memory[i + 2]);
            args.push_back(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 2]);
            two_operand_next(i);
            break;
        case OP_CALL: /* have control over position */
            this->_call(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 1]);
            break;
        case OP_RET: /* have control over position */
            this->_ret();
            break;
        case OP_OUT:
            this->_out(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 1]);
            one_operand_next(i);
            break;
        case OP_IN:
            this->_in(this->_memory[i + 1]);
            args.push_back(this->_memory[i + 1]);
            one_operand_next(i);
            break;
        case OP_NOOP:
            this->_noop();
            no_operand_next(i);
            break;

        default: /* raw memory: not an operation, just data */
            no_operand_next(i);
            break;
        }

        if(is_op(op_code)) {
            this->ops[curr_addr] = new Op(curr_addr, (OpCode) op_code, i, args);

            if(!args.size())
                args.clear();
        }

        if(i >= msize) {
            std::cerr << "Out of memory" << std::endl;
            this->state = VM_ERR_OUT_OF_MEMORY;
        }
    }
    
    if(this->state == VM_ERR)
        return false;
    
    return true;
};


/***********************************************/
/*                   Helpers                   */
/***********************************************/

inline void VirtualMachine::check_val(uint16_t& val) {
    if (val >= INVALID_LOW_END) {
        std::cerr << "invalid data:" << val << std::endl;
        this->state = VM_ERR;
        return;
    }
}

inline void VirtualMachine::check_val(const uint16_t& val) {
    VirtualMachine::check_val((uint16_t&) val);
}

inline void VirtualMachine::cast_val(uint16_t &a, const uint16_t &val) {
    if(val >= REG_LOW)
        a = this->_registers[val % REG_LOW];
    else
        a = val;
}

inline uint16_t& VirtualMachine::cast_res(uint16_t& val) {
    if(val >= REG_LOW)
        return this->_registers[val % REG_LOW];
    else
        return val;
}

inline void set_next(uint16_t& index, const uint16_t& target) {
    index = target;
}

inline void no_operand_next(uint16_t& index) {
    set_next(index, index + 1);
}

inline void one_operand_next(uint16_t& index) {
    set_next(index, index + 2);
}

inline void two_operand_next(uint16_t& index) {
    set_next(index, index + 3);
}

inline void three_operand_next(uint16_t& index) {
    set_next(index, index + 4);
}

/***********************************************/
/*                 Operations                  */
/***********************************************/
void VirtualMachine::_halt() {
    this->state = VM_HALT;
}

void VirtualMachine::_set(uint16_t& res, const uint16_t& val) {
    check_val(res);
    check_val(val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0;
    
    cast_val(b, val);

    a = b;
}

void VirtualMachine::_push(const uint16_t& val) {
    check_val(val);

    uint16_t a = 0;
    
    cast_val(a, val);

    this->_stack.push(a);
}

void VirtualMachine::_pop(uint16_t& res) {
    if(!this->_stack.size()) {
        std::cerr << "empty stack" << std::endl;
        this->state = VM_ERR;
        return;
    }

    check_val(res);

    uint16_t& a = cast_res(res);

    a = this->_stack.top();
    this->_stack.pop();
}

void VirtualMachine::_eq(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val) {
    check_val(res);
    check_val(l_val);
    check_val(r_val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0, c = 0;
    
    cast_val(b, l_val);
    cast_val(c, r_val);

    if (b == c)
        a = 1;
    else
        a = 0;
}

void VirtualMachine::_gt(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val) {
    check_val(res);
    check_val(l_val);
    check_val(r_val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0, c = 0;

    cast_val(b, l_val);
    cast_val(c, r_val);

    if (b > c)
        a = 1;
    else
        a = 0;
}

void VirtualMachine::_jmp(const uint16_t& j_val) {
    check_val(j_val);

    uint16_t a =  0;

    cast_val(a, j_val);

    set_next(this->pos, a);
}

void VirtualMachine::_jt(const uint16_t& val, const uint16_t& j_val) {
    check_val(val);
    check_val(j_val);

    uint16_t a = 0, b = 0;

    cast_val(a, val);
    cast_val(b, j_val);

    if(a)
        set_next(this->pos, b);
    else
        two_operand_next(this->pos);
}

void VirtualMachine::_jf(const uint16_t& val, const uint16_t& j_val) {
    check_val(val);
    check_val(j_val);

    uint16_t a = 0, b = 0;

    cast_val(a, val);
    cast_val(b, j_val);

    if(!a)
        set_next(this->pos, b);
    else
        two_operand_next(this->pos);
}

void VirtualMachine::_add(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val) {
    check_val(res);
    check_val(l_val);
    check_val(r_val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0, c = 0;

    cast_val(b, l_val);
    cast_val(c, r_val);

    a = (b + c) % MODULO;
}

void VirtualMachine::_mult(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val) {
    check_val(res);
    check_val(l_val);
    check_val(r_val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0, c = 0;

    cast_val(b, l_val);
    cast_val(c, r_val);

    a = (b * c) % MODULO;
}

void VirtualMachine::_mod(uint16_t& res, const uint16_t& l_val,
    const uint16_t& r_val) {

    check_val(res);
    check_val(l_val);
    check_val(r_val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0, c = 0;

    cast_val(b, l_val);
    cast_val(c, r_val);

    a = b % c;
}

void VirtualMachine::_and(uint16_t& res, const uint16_t& l_val,
    const uint16_t& r_val) {

    check_val(res);
    check_val(l_val);
    check_val(r_val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0, c = 0;

    cast_val(b, l_val);
    cast_val(c, r_val);

    a = b & c;
}

void VirtualMachine::_or(uint16_t& res, const uint16_t& l_val,
    const uint16_t& r_val) {

    check_val(res);
    check_val(l_val);
    check_val(r_val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0, c = 0;

    cast_val(b, l_val);
    cast_val(c, r_val);

    a = b | c;
}

void VirtualMachine::_not(uint16_t& res, const uint16_t& val) {
    check_val(res);
    check_val(val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0;

    cast_val(b, val);

    a = ((uint16_t) ~b) % MODULO;
}

void VirtualMachine::_rmem(uint16_t& res, const uint16_t& val) {
    check_val(res);
    check_val(val);

    uint16_t& a = cast_res(res);
    uint16_t b = 0;

    cast_val(b, val);

    a = this->_memory[b];
}

void VirtualMachine::_wmem(uint16_t& res, const uint16_t& val) {
    check_val(res);
    check_val(val);

    uint16_t a = 0, b = 0;

    cast_val(a, res);
    cast_val(b, val);

    this->_memory[a] = b;
}

void VirtualMachine::_call(const uint16_t& j_val) {
    check_val(j_val);

    one_operand_next(this->pos);
    this->_stack.push(this->pos);

    uint16_t a = 0;

    cast_val(a, j_val);

    set_next(this->pos, a);
}

void VirtualMachine::_ret() {
    if(!this->_stack.size()) {
        this->state = VM_HALT;
        return;
    }

    set_next(this->pos, this->_stack.top());
    this->_stack.pop();
}

void VirtualMachine::_out(const uint16_t& val) {
    check_val(val);

    uint16_t a = 0;

    cast_val(a, val);

    std::cout << (char) a;
}

void VirtualMachine::_in(uint16_t& res) {
    check_val(res);

    uint16_t& a = cast_res(res);

    a = (char) getchar();
}

void VirtualMachine::_noop() {
    /* nothing */
}
