#ifndef OPERATORS_H
#define OPERATORS_H
#include "Generator.h"

class Op;
class Emulator;

enum Operator {
    ADD,
    SUBTRACT,
    MULT,
    DIV,
    EQ,
    NEQ,
    AND,
    OR,
    GRTR,
    GRTR_EQ,
    LESS,
    LESS_EQ,
    STRUCT_DOT,
    STRUCT_REF
};

class TypeOperator: public Op {
    public:
        SerializedType type;
        int dataSize() {
            return sizeof(type);
        }
        void putData(char *data) {
            memcpy(data, &type, sizeof(type));
        }
        void parseData(char *data) {
            memcpy(&type, data, sizeof(type));
        }
        virtual void run(Emulator &e) {}
};

class Negative: public TypeOperator {
    public:
        Negative() {
            code = OP_NEG;
        }
        Negative(SerializedType t) {
            code = OP_NEG;
            type = t;
        }
        virtual void run(Emulator &e);
};

class Not: public Op {
    public:
        Not() {
            code = OP_NOT;
        }
        virtual void run(Emulator &e);
};

#define TYPE_OP_CLASS(name, opcode, acceptsPtrs) \
class name: public TypeOperator{\
public:\
    bool acceptPtr;\
    name(){\
        acceptPtr = acceptsPtrs;\
        code = opcode;\
    }\
    name(SerializedType t) {\
        acceptPtr = acceptsPtrs;\
        code = opcode;\
        type = t;\
    }\
    void ptrRun(Emulator &e);\
    virtual void run(Emulator &e);\
}\

TYPE_OP_CLASS(Add, op_add, true);
TYPE_OP_CLASS(Sub, op_sub, true);
TYPE_OP_CLASS(Mult, op_mult, false);
TYPE_OP_CLASS(Div, op_div, false);

TYPE_OP_CLASS(CmpEq, op_eq, true);
TYPE_OP_CLASS(CmpNeq, op_neq, true);

TYPE_OP_CLASS(CmpGrtr, op_grtr, false);
TYPE_OP_CLASS(CmpGrtrEq, op_grteq, false);
TYPE_OP_CLASS(CmpLess, op_lessthan, false);
TYPE_OP_CLASS(CmpLessEq, op_leseq, false);

TYPE_OP_CLASS(And, op_and, false);
TYPE_OP_CLASS(Or, op_or, false);


#endif
