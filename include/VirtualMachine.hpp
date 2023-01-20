#ifndef _VIRTUAL_MACHINE_HPP
#define _VIRTUAL_MACHINE_HPP

/* MATH */
#define MAX_LITERAL     32767
#define MODULO          32768

/* MEMORY */
#define MEMORY_SIZE     MAX_LITERAL

/* REGISTERS */
#define REG_COUNT       8
#define REG_LOW         (MAX_LITERAL + 1)
#define REG_HIGH        (REG_LOW + REG_COUNT)

#define INVALID_LOW_END (REG_HIGH + 1)

#include <cinttypes>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <fstream>

#include "Op.hpp"

using namespace std;

enum VMState {
    VM_INIT = 0,
    VM_RUN,
    VM_PLAYING_FILE,
    VM_HALT,
    VM_ERR,
    VM_ERR_BAD_MEMORY,
};

inline void no_operand_next(uint16_t& index);
inline void one_operand_next(uint16_t& index);
inline void two_operand_next(uint16_t& index);
inline void three_operand_next(uint16_t& index);
inline void set_next(uint16_t& index, const uint16_t& target);

class VirtualMachine {
public:
    VirtualMachine(VirtualMachine &) = delete; /* prevent cloning */
    void operator=(const VirtualMachine &) = delete;
    static VirtualMachine *get();

    bool run(string& s_path);
    void extractText(string& d_path);
    void dumpAsm(string& d_path);

    void set_playfile(string &p_path);

protected:
    static VirtualMachine *vm; /* Singleton */
    VirtualMachine();

    /* signal handling */
    static void s_interrupt(int sig) {
        vm->interrupt(sig);
    }
    void interrupt(int sig);
    void hack_mode();

private:
    VMState state;

    uint16_t pos;
    map<uint16_t, uint16_t> mem_hacks;
    map<uint16_t, uint16_t> reg_hacks;

    vector<uint16_t> _memory;
    vector<uint16_t> _registers;
    stack<uint16_t> _stack;

    std::ifstream ifs;
    std::streambuf *cin_backup;

    /* helpers */
    bool loadFile(string& s_path);
    inline void check_val(uint16_t& val);
    inline void check_val(const uint16_t& val);
    inline void cast_val(uint16_t& a, const uint16_t& val);
    inline uint16_t& cast_res(uint16_t& val);

    /* Ops as listed in the spec */
    void _halt();
    void _set(uint16_t& res, const uint16_t& val);
    void _push(const uint16_t& val);
    void _pop(uint16_t& res);
    void _eq(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val);
    void _gt(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val);
    void _jmp(const uint16_t& j_val);
    void _jt(const uint16_t& val, const uint16_t& j_val);
    void _jf(const uint16_t& val, const uint16_t& j_val);
    void _add(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val);
    void _mult(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val);
    void _mod(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val);
    void _and(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val);
    void _or(uint16_t& res, const uint16_t& l_val, const uint16_t& r_val);
    void _not(uint16_t& res, const uint16_t& val);
    void _rmem(uint16_t& res, const uint16_t& val);
    void _wmem(uint16_t& res, const uint16_t& val);
    void _call(const uint16_t& j_val);
    void _ret();
    void _out(const uint16_t& val);
    void _in(uint16_t& res);
    void _noop();
};

#endif /* _VIRTUAL_MACHINE_HPP */