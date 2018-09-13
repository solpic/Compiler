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

class Parser{
private:
    int labelMain;
    
    Tokenizer *t;
    SymTab sym;
    Generator g;
    static std::ostream *pout;
    
    void declarations(bool funcs);
    void decl();
    void decl_tail();
    void var_decl();
    void match(const char *s);
    bool testMatch(const char *s);
    Function *currentFunc;
    Token* cur();
    Token* next();
    void matchType(TokenType type, const char *expected);
    bool testMatchType(TokenType type);

    void func_decl();
    void func_def();

    void statements();
    void assignment();
    void pointerAssignment();
    
    Type typecast();
    Type E();
    Type T();
    Type F();
    void function(Function *f);
    void throwError();
    
    bool matchOperatorE(Operator &op);
    bool matchOperatorT(Operator &op);
    
    void controlStatement(Symbol *s);
    
    ControlType getControlType();
    void builtInFunction(Symbol *s);

public:
    static void setOutputStream(std::ostream *o) {
        pout = o;
    }
    ~Parser() {}
    Parser(Tokenizer *tok);
    void parse();
    void compile(const char *fname);
    
    void addVar(std::string name, Type t);
};


#endif
