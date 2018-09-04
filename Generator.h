#ifndef GENERATOR_H
#define GENERATOR_H

#include <list>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "Error.h"
#include "SymTab.h"

#define op_pushi 1
#define op_halt 2
#define op_pop 3
#define op_printint 4
#define op_push 5
#define op_print 6
#define op_ifnjmp 7 //if not jump
#define op_label 8 //not a real op, just a placeholder
#define op_add 9
#define op_sub 10
#define op_mult 11
#define op_div 12
#define op_and 13
#define op_or 14
#define op_eq 15
#define op_neq 16
#define op_readint 17
#define op_leseq 18
#define op_lessthan 19
#define op_grteq 20
#define op_grtr 21
#define op_jmp 22
#define op_pushlbl 23
#define OP_RETURN 24
#define OP_CALL 25
#define OP_REFPTR 26
#define OP_POPPTR 27
#define OP_CAST 28
#define OP_NEG 29
#define OP_NOT 30

class SymTab;

#define OPSIZE sizeof(unsigned int)*2
class Emulator;

class Op{
public:
    int code;
    Op(){ code = -1;}
    virtual ~Op(){}
    virtual void putData(char *data) {}
    virtual void parseData(char *data) {}
    virtual int dataSize() { return 0; }
    virtual int opSize() {
        return sizeof(code) + dataSize();
    }
    virtual void run(Emulator &e) {
        std::cout<<"No run code for op "<<code<<std::endl;
        error();
    }
    
    static Op* opFromCode(int code);
};

class Label: public Op{
public:
    LBL lbl;
    Label(){ code = op_label; }
    Label(LBL l) {
        code = op_label;
        lbl = l;
    }
};

class Return: public Op{
public:
    Return() { code = OP_RETURN; }
    void run(Emulator &e);
};

class Call: public Op{
public:
    Call() { code = OP_CALL; }
    void run(Emulator &e);
};

class Jump: public Op{
public:
    Jump() { code = op_jmp; }
    void run(Emulator &e);
};

class PushLbl: public Op{
public:
    LBL lbl;
    PushLbl() { code = op_pushlbl; }
    PushLbl(LBL l) {
        code = op_pushlbl;
        lbl = l;
    }
    void run(Emulator &e);
    int dataSize() { return LBL_SIZE; }
    void putData(char *data) {
        memcpy(data, &lbl, LBL_SIZE);
    }
    void parseData(char *data) {
        memcpy(&lbl, data, LBL_SIZE);
    }
};

class Typecast: public Op{
public:
    SerializedType to;
    SerializedType from;
    Typecast() { code = OP_CAST; }
    Typecast(SerializedType t, SerializedType f) {
        to = t;
        from = f;
        code = OP_CAST;
    }
    void run(Emulator &e);
    int dataSize() { return sizeof(to)+sizeof(from); }
    void putData(char *data) {
        memcpy(data, &to, sizeof(to));
        memcpy(data+sizeof(to), &from, sizeof(from));
    }
    void parseData(char *data) {
        memcpy(&to, data, sizeof(to));
        memcpy(&from, data+sizeof(to), sizeof(from));
    }
};

class Pop: public Op{ 
public:
    int addr, size;
    Pop(){ code = op_pop; }
    Pop(int a, int s) {
        addr = a;
        size = s;
        code = op_pop;
    }
    virtual void run(Emulator &e);
    int dataSize() { return sizeof(addr)+sizeof(size); }
    void putData(char *data) {
        memcpy(data, &addr, sizeof(addr));
        memcpy(data+sizeof(addr), &size, sizeof(size));
    }
    void parseData(char *data) {
        memcpy(&addr, data, sizeof(addr));
        memcpy(&size, data+sizeof(addr), sizeof(size));
    }
};

class Push: public Pop{
public:
    Push() { code = op_push; }
    Push(int a, int s) {
        addr = a;
        size = s;
        code = op_push;
    }
    void run(Emulator &e);
};

//Top of stack is value, next is pointer address
//Pop value into pointer address
class PopToPtr: public Op{
public:
    int size;
    PopToPtr() { code = OP_POPPTR; }
    PopToPtr(int s) {
        code = OP_POPPTR;
        size = s;
    }
    int dataSize() { return sizeof(size); }
    void putData(char *data) {
        memcpy(data, &size, sizeof(size));
    }
    void parseData(char *data) {
        memcpy(&size, data, sizeof(size));
    }
    void run(Emulator &e);
};

//Top of stack is pointer address, pop it and push value onto stack
class PushPtr: public Op{
public:
    int size;
    PushPtr() { code = OP_REFPTR; }
    PushPtr(int s) {
        code = OP_REFPTR;
        size = s;
    }
    int dataSize() { return sizeof(size); }
    void putData(char *data) {
        memcpy(data, &size, sizeof(size));
    }
    void parseData(char *data) {
        memcpy(&size, data, sizeof(size));
    }
    void run(Emulator &e);
};

class Halt: public Op{
public:
    Halt() { code = op_halt; }
    void run(Emulator &e){}
};

class PrintNum: public Op{
public:
    SerializedType type;
    PrintNum() { code = op_printint; }
    PrintNum(SerializedType v) {
        type = v;
        code = op_printint;
    }
    int dataSize() { return sizeof(type); }
    void putData(char *data) {
        memcpy(data, &type, sizeof(type));
    }
    void parseData(char *data) {
        memcpy(&type, data, sizeof(type));
    }
    void run(Emulator &e);
};

class PushI: public Op{
public:
    int size;
    char *value;
    PushI() { code = op_pushi; }
    PushI(int s, void *v) {
        size = s;
        value = new char[size];
        memcpy(value, v, size);
        code = op_pushi;
    }
    ~PushI() { delete[] value; }
    int dataSize() { return sizeof(size) + size; }
    void putData(char *data) {
        memcpy(data, &size, sizeof(size));
        memcpy(data+sizeof(size), value, size);
    }
    void parseData(char *data) {
        memcpy(&size, data, sizeof(size));
        value = new char[size];
        memcpy(value, data+sizeof(size), size);
    }
    void run(Emulator &e);
};

class Generator{
private:
    std::list<Op*> ops;
public:
    int curLabel;
    Generator() { curLabel = 0;}
    ~Generator() {}
    void resolveLabels(int dataSize);
    void addOp(Op *o) { ops.push_back(o); }
    void generate(const char *fname, int dataSize, SymTab &sym);
};

class Emulator{
public:
    char* code;
    static std::string prefix;
    int ip, data;
    int sp;
    std::vector<char> stk;
    std::list<int> stkSizes;    //Sizes of stack elements, for debugging purposes
    void push(int size, void *val);
    void pop(int size, void *val);
    
    static std::ostream *eout;
    void printStack();
    static void setOutput(std::ostream *o) { eout = o; }
    Emulator(const char *fname);
    ~Emulator();
    void run();
    void runOp(Op &o);
};

#endif
