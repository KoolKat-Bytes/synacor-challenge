#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <cinttypes>
#include <csignal>
#include <limits>

#include "Op.hpp"
#include "VirtualMachine.hpp"

using namespace std;

VirtualMachine* VirtualMachine::vm = VirtualMachine::get();

VirtualMachine* VirtualMachine::get() {
    if (!vm)
        vm = new VirtualMachine();

    return vm;
}

VirtualMachine::VirtualMachine() {
    signal(SIGINT, &VirtualMachine::s_interrupt);

    this->state = VM_INIT;
    this->pos = 0;
    this->_registers = vector<uint16_t>(REG_COUNT);
}

void VirtualMachine::extractText(string& d_path) {
    ofstream os;

    os.open(d_path);

    if(!os.is_open()) {
        cerr << "failed to open: '" << d_path << "'" << endl;
        return;
    }

    /* 33 -> 126 printable chars and 10 is new line */
    for(auto const& it : this->_memory) {
        if(it >= 32 && it <= 126 || it == 10)
            os << (char)it;
    }
    os.close();
}

void VirtualMachine::dumpAsm(string& d_path) {
    ofstream os;

    os.open(d_path);

    if(!os.is_open()) {
        cerr << "failed to open: '" << d_path << "'" << endl;
        return;
    }

    /* current position of the program */
    os << "pos: " << this->pos << endl;

    os << setfill('=') << setw(24)
       << "registers" << endl;
    for(auto const &it: this->_registers)
        os << it << ", ";
    os << endl;

    constexpr int stack_count = 10;
    os << setfill('=') << setw(24)
       << "stack" << endl;

    /* last <stack_count> elements (if any) of the stack */
    stack<uint16_t> stmp;
    int ssize = this->_stack.size();
    for(int i = 0; i < ssize && i < stack_count; i++) {
        stmp.push(this->_stack.top());
        this->_stack.pop();
    }
    while (!stmp.empty()) {
        uint16_t top = stmp.top();
        os << top << ",";
        this->_stack.push(top);
        stmp.pop();
    }
    os << endl;

    /* print memory */
    string ops;
    string mem;
    stringstream ssops(ops);
    stringstream ssmem(mem);

    size_t msize = this->_memory.size();
    for(int i = 0; i < msize; i++) {
        if (is_op(this->_memory[i])) {
            ssops << "[" << setfill(' ') << setw(5) << i << "] ";
            i += decode_op(ssops, this->_memory, i);
            ssops << endl;
        } else {
            ssmem << "[" << setfill(' ') << setw(5) << i << "] ";
            ssmem << " mem " << this->_memory[i];
            /* 33 -> 126 printable chars and 10 is new line */
            if(this->_memory[i] >= 33 && this->_memory[i] <= 126) {
                ssmem << setfill(' ') << setw(20);
                ssmem << "; " << (char)this->_memory[i];
            }
            ssmem << endl;
        }
    }


    os << setfill('=') << setw(24)
       << "ops" << endl
       << ssops.str();

    os << setfill('=') << setw(24)
       << "memory" << endl
       << ssmem.str();


    os.close();
}

void VirtualMachine::set_playfile(string& p_path) {
    /* only available while not run */
    if (this->state != VM_INIT)
        return;

    this->ifs = ifstream(p_path, ifstream::in);

    if (!this->ifs.is_open()) {
        cerr << "Error opening:'" << p_path << "'" <<  endl;
        return;
    }

    this->state = VM_PLAYING_FILE;

    this->cin_backup= cin.rdbuf(this->ifs.rdbuf());
}

void VirtualMachine::hack_mode() {
    this->interrupt(SIGINT);
}

void VirtualMachine::interrupt(int sig) {
    if (sig != SIGINT)
        return;

    if (this->cin_backup && cin.eof()) {
        cin.rdbuf(this->cin_backup);
        this->cin_backup = nullptr;
    }

    int i = 0;
    string cmd;

    do {
        cout << endl;
        cout << "### Hacking at YOUR OWN RISK ###" << endl;
        cout << "Available commands:" << endl;
        cout << "\t-get <'reg'|'mem'> <pos> <count>" << endl;
        cout << "\t-set <'reg'|'mem'> <pos> <value>" << endl;
        cout << "\t-mdump <filename>" << endl;
        cout << "\t-text <filename>" << endl;
        cout << "\t-terminate" << endl;
        cout << "\t-! (continue execution)" << endl;
        cout << "your command ?: ";

        string line;
        getline(cin, line);

        if (this->state == VM_PLAYING_FILE)
            cout << line << endl;

        stringstream ss(line);
        string target, first, second;
        getline(ss, cmd, ' ');
        getline(ss, target, ' ');
        getline(ss, first, ' ');
        getline(ss, second, ' ');

        int fn = 0, sn = 7;

        if (!first.empty())
            fn = stoi(first.c_str());

        if (!second.empty())
            sn = stoi(second.c_str());

        if (!cmd.compare("get")) {
            if (!target.compare("mem")) {
                const int range = fn + sn;
                for (int i = fn; i < range; i++)
                    cout << to_string(this->_memory[i]) << ", ";
                cout << endl;
            } else if (!target.compare("reg")) {
                for (const auto& it : this->_registers)
                    cout << to_string(it) << ", ";
                cout << endl;
            }
        }

        if (!cmd.compare("set")) {
            if (!target.compare("mem")) {
                this->_memory[fn] = sn;
                this->mem_hacks.emplace(make_pair(fn ,sn));
            }

            if (!target.compare("reg")) {
                this->_registers[fn] = sn;
                this->reg_hacks.emplace(make_pair(fn ,sn));
            }
        }

        if (!cmd.compare("mdump")) {
            if (target.empty()) {
                cerr << "please set <filename>" << endl;
                goto next;
            }

            /* trusting the user to provide a sane path */
            cout << "Dumping memory to " << "'" << target << "' ..." << endl;
            this->dumpAsm(target);
        }

        if (!cmd.compare("text")) {
            if (target.empty()) {
                cerr << "please set <filename>" << endl;
                goto next;
            }

            /* trusting the user to provide a sane path */
            cout << "Extracting text to " << "'" << target << "' ..." << endl;
            this->extractText(target);
        }

        if (!cmd.compare("terminate")) {
            /* default behavior */
            signal(sig, SIG_DFL);
            raise(SIGINT);
            return;
        }

    next:
        cout << "'" << cmd << "'" << " processed !" << endl;
        line.clear();
    } while (cmd.compare("!"));

    /* None of this happened (¬ ‿¬) */
    cin.clear();
    cin.putback('\n');
    cin.sync();
}

bool VirtualMachine::loadFile(string& s_path) {
    ifstream is;

    is.open(s_path, ios::binary);

    if(!is.is_open()) {
        cerr << "failed to open: '" << s_path << "'" << endl;
        return false;
    }

    streampos fsize;

    is.seekg(0, ios::end);
    fsize = is.tellg();
    is.seekg(0, ios::beg);

    // NOTE: assuming file size is no more than MEMORY SIZE
    // TODO: read chunk by chunk otherwise
    this->_memory.resize(fsize / sizeof(uint16_t));
    is.read((char*)(this->_memory.data()), fsize);

    is.close();

    return true;
}

bool VirtualMachine::run(string& s_path) {

    if(!this->loadFile(s_path)) {
        cerr << "Failed to load file to the virtual machine" << endl;
        return false;
    }

    /* start execution */
    if (this->state == VM_INIT)
        this->state = VM_RUN;

    const size_t msize = this->_memory.size();
    uint16_t &i = this->pos;

    while(this->state == VM_RUN || this->state == VM_PLAYING_FILE) {

        auto lookup = this->mem_hacks.find(i);
        if (lookup != this->mem_hacks.end()) {
            this->_memory[i] = lookup->second;
            this->mem_hacks.erase(lookup);

            for (auto rh = this->reg_hacks.begin();
                    rh != this->reg_hacks.end(); rh++) {

                this->_registers[rh->first] = rh->second;
            }
            this->reg_hacks.clear();
        }

        switch(this->_memory[i])
        {
        case OP_HALT:
            this->_halt();
            no_operand_next(i);
            break;

        case OP_SET:
            this->_set(this->_memory[i + 1], this->_memory[i + 2]);
            two_operand_next(i);
            break;

        case OP_PUSH:
            this->_push(this->_memory[i + 1]);
            one_operand_next(i);
            break;

        case OP_POP:
            this->_pop(this->_memory[i + 1]);
            one_operand_next(i);
            break;

        case OP_EQ:
            this->_eq(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            three_operand_next(i);
            break;

        case OP_GT:
            this->_gt(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            three_operand_next(i);
            break;

        case OP_JMP: /* have control over position */
            this->_jmp(this->_memory[i + 1]);
            break;

        case OP_JT: /* have control over position */
            this->_jt(this->_memory[i + 1], this->_memory[i + 2]);
            break;

        case OP_JF: /* have control over position */
            this->_jf(this->_memory[i + 1], this->_memory[i + 2]);
            break;

        case OP_ADD:
            this->_add(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            three_operand_next(i);
            break;

        case OP_MULT:
            this->_mult(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            three_operand_next(i);
            break;

        case OP_MOD:
            this->_mod(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            three_operand_next(i);
            break;

        case OP_AND:
            this->_and(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            three_operand_next(i);
            break;

        case OP_OR:
            this->_or(this->_memory[i + 1], this->_memory[i + 2], this->_memory[i + 3]);
            three_operand_next(i);
            break;

        case OP_NOT:
            this->_not(this->_memory[i + 1], this->_memory[i + 2]);
            two_operand_next(i);
            break;

        case OP_RMEM:
            this->_rmem(this->_memory[i + 1], this->_memory[i + 2]);
            two_operand_next(i);
            break;

        case OP_WMEM:
            this->_wmem(this->_memory[i + 1], this->_memory[i + 2]);
            two_operand_next(i);
            break;

        case OP_CALL: /* have control over position */
            this->_call(this->_memory[i + 1]);
            break;

        case OP_RET: /* have control over position */
            this->_ret();
            break;

        case OP_OUT:
            this->_out(this->_memory[i + 1]);
            one_operand_next(i);
            break;

        case OP_IN:
            this->_in(this->_memory[i + 1]);
            one_operand_next(i);
            break;

        case OP_NOOP:
            this->_noop();
            no_operand_next(i);
            break;

        default: /* while running the program we should never be here */
            this->state = VM_ERR_BAD_MEMORY;
            continue;
        }
    }

    if (this->state != VM_HALT)
        return false;

    return true;
};


/***********************************************/
/*                   Helpers                   */
/***********************************************/

inline void VirtualMachine::check_val(uint16_t& val) {
    if (val >= INVALID_LOW_END) {
        cerr << "invalid data:" << val << endl;
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
        cerr << "empty stack" << endl;
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

    cout << (char) a;
}

void VirtualMachine::_in(uint16_t& res) {
    check_val(res);

    uint16_t& a = cast_res(res);

    if (this->state == VM_PLAYING_FILE) {
        cout << (char) cin.peek();

        if (this->cin_backup && cin.eof()) {
            cin.rdbuf(this->cin_backup);
            this->cin_backup = nullptr;

            this->state = VM_RUN;
            cin.clear();
            cin.putback('\n');
            cin.sync();
        }
    }

    if (a == '!') /* hack mode */
        this->hack_mode();

    if (a == '#') /* comments */
        cin.ignore(numeric_limits<streamsize>::max(), '#');

    a = (char) cin.get();
}

void VirtualMachine::_noop() {
    /* nothing */
}
