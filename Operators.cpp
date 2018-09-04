#include "Operators.h"
#include <iostream>
#include "SymTab.h"

using namespace std;

template <class T>
void negOp(Emulator &e) {
    T a;
    e.pop(sizeof(T), &a);
    a = -a;
    e.push(sizeof(T), &a);
}

void Negative::run(Emulator &e) {
    if(type==BIN_DBL) {
        negOp<t_dbl>(e);
    }else if(type==BIN_INT) {
        negOp<t_int>(e);
    }else{
        cout<<"NEG: Unrecognized type"<<endl;
        error();
    }
    
    e.ip += opSize();
}

void Not::run(Emulator &e) {
    t_char a;
    e.pop(sizeof(a), &a);
    a = !a;
    e.push(sizeof(a), &a);
    
    e.ip += opSize();
}

template <class A, class B, class C>
void binaryOperator(Emulator &e, C (*b_op)(A, B)) {
    A a;
    B b;
    C c;
    e.pop(sizeof(B), &b);
    e.pop(sizeof(A), &a);
    
    c = b_op(a, b);
    e.push(sizeof(C), &c);
}

/*
 * Macro to handle typing for binary operators that return a type equal to the initial types
 * Such as +, -, *, etc
 * Only some take pointers
 */
 
#define OP_RUN_DEF(cname, op, name)\
void cname::run(Emulator &e) {\
    if(type==BIN_CHAR)\
        binaryOperator<t_char, t_char, t_char>(e, op<t_char, t_char, t_char>);\
    else if(type==BIN_DBL) \
        binaryOperator<t_dbl, t_dbl, t_dbl>(e, op<t_dbl, t_dbl, t_dbl>);\
    else if(type==BIN_INT)\
        binaryOperator<t_int, t_int, t_int>(e, op<t_int, t_int, t_int>);\
    else if(type==BIN_PTR&&acceptPtr)\
        ptrRun(e);\
    else{\
        cout<<name<<": Unrecognized type "<<Type::toString(type)<<endl;\
        error();\
    }\
    e.ip += opSize();\
}\


/*
 * For those exceptions where the return type is not the same as the initial type,
 * i.e. ==, !=, >= ...
 * Notably these will all accept pointers
 */
 
#define OP_RUN_DEF_RET_CHAR(cname, op, name)\
void cname::run(Emulator &e) {\
    if(type==BIN_CHAR)\
        binaryOperator<t_char, t_char, t_char>(e, op<t_char, t_char, t_char>);\
    else if(type==BIN_DBL) \
        binaryOperator<t_dbl, t_dbl, t_char>(e, op<t_dbl, t_dbl, t_char>);\
    else if(type==BIN_INT)\
        binaryOperator<t_int, t_int, t_char>(e, op<t_int, t_int, t_char>);\
    else if(type==BIN_PTR&&acceptPtr)\
        binaryOperator<t_ptr, t_ptr, t_char>(e, op<t_ptr, t_ptr, t_char>);\
    else{\
        cout<<name<<": Unrecognized type"<<endl;\
        error();\
    }\
    e.ip += opSize();\
}\
void cname::ptrRun(Emulator &e) {}\

#define EMPTY_PTR_RUN(name) \
void name::ptrRun(Emulator &e) {}

#define OP_TMP_DEF(name, op) \
template <class A, class B, class C>\
C name(A a, B b) {\
    return a op b;\
}\

#define OP_TMP_DEF_CASTED(name, op) \
template <class A, class B, class C>\
C name(A a, B b) {\
    return (C)(a op b);\
}\
/*
 * Define addition
 */
OP_TMP_DEF(addOp, +)
void Add::ptrRun(Emulator &e) {
    binaryOperator<t_ptr, t_int, t_ptr>(e, addOp<t_ptr, t_int, t_ptr>);
}
OP_RUN_DEF(Add, addOp, "ADD")

/*
 * Define subtraction
 */
OP_TMP_DEF(subOp, -)
void Sub::ptrRun(Emulator &e) {
    binaryOperator<t_ptr, t_int, t_ptr>(e, subOp<t_ptr, t_int, t_ptr>);
}
OP_RUN_DEF(Sub, subOp, "SUB")

//Define multiplication
OP_TMP_DEF(multOp, *)
EMPTY_PTR_RUN(Mult)
OP_RUN_DEF(Mult, multOp, "MULT")

//Define division
OP_TMP_DEF(divOp, /)
EMPTY_PTR_RUN(Div)
OP_RUN_DEF(Div, divOp, "DIV")

//Define ==
OP_TMP_DEF_CASTED(eqOp, ==)
OP_RUN_DEF_RET_CHAR(CmpEq, eqOp, "EQ")

//Define !=
OP_TMP_DEF_CASTED(neqOp, !=)
OP_RUN_DEF_RET_CHAR(CmpNeq, neqOp, "NEQ")

//Define >
OP_TMP_DEF_CASTED(grtrOp, >)
OP_RUN_DEF_RET_CHAR(CmpGrtr, grtrOp, "GRTR")
//Define >=
OP_TMP_DEF_CASTED(grtrEqOp, >=)
OP_RUN_DEF_RET_CHAR(CmpGrtrEq, grtrEqOp, "GRTREQ")
//Define <
OP_TMP_DEF_CASTED(lesOp, <)
OP_RUN_DEF_RET_CHAR(CmpLess, lesOp, "LESS")
//Define <=
OP_TMP_DEF_CASTED(lesEqOp, <=)
OP_RUN_DEF_RET_CHAR(CmpLessEq, lesEqOp, "LESSEQ")

//Define && (only for char)
OP_TMP_DEF(andOp, &&)
EMPTY_PTR_RUN(And)
void And::run(Emulator &e) {
    if(type==BIN_CHAR) {
        binaryOperator<t_char, t_char, t_char>(e, andOp<t_char, t_char, t_char>);
    }else{
        cout<<"AND: Unrecognized type"<<endl;
        error();
    }
    
    e.ip += opSize();
}

//Define || (only for char)
OP_TMP_DEF(orOp, ||)
EMPTY_PTR_RUN(Or)
void Or::run(Emulator &e) {
    if(type==BIN_CHAR) {
        binaryOperator<t_char, t_char, t_char>(e, orOp<t_char, t_char, t_char>);
    }else{
        cout<<"OR: Unrecognized type"<<endl;
        error();
    }
    
    e.ip += opSize();
}
