#ifndef PARSER_H
#define PARSER_H
#include "Tokenizer.h"
#include "SymTab.h"
#include "Generator.h"
#include "Operators.h"
#include <iostream>
#include <ostream>
#include <list>
#include <algorithm>
#include <utility>

enum ControlType{
    If = 1,
    While = 2
};

class Parser{
private:
    int labelMain;
    
    Tokenizer *t;
    SymTab sym;
    Generator g;
    std::string asmOut;
    static std::ostream *pout;
    
    void procedures();
    
    void declarations(bool funcs);
    void decl();
    void decl_tail();
    void var_decl(VarType varType);
    void match(const char *s);
    bool testMatch(const char *s);
    Symbol *currentFunc;
    Token* cur();
    Token* next();
    void matchType(TokenType type, const char *expected);
    bool testMatchType(TokenType type);

    void func_decl();
    void func_def();

    void main();
    void statements();
    void assignment();
    void pointerAssignment();
    
    ParseVar typecast();
    ParseVar E();
    ParseVar T();
    ParseVar F();
    ParseVar function(Symbol &s);
    
    bool matchOperatorE(Operator &op);
    bool matchOperatorT(Operator &op);
    ParseVar parseFunctionType();
    
    void controlStatement(Symbol &s);
    
    ControlType getControlType();
    void builtInFunction(Symbol &s);

public:
    static void setOutputStream(std::ostream *o) {
        pout = o;
    }
    ~Parser() {}
    Parser(Tokenizer *tok);
    void parse();
    void dumpAsm() { std::cout<<asmOut; }
    void compile(const char *fname);
};


#endif
