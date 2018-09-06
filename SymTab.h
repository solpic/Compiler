#ifndef SYMTAB_H
#define SYMTAB_H

#include <unordered_map>
#include <string>
#include <list>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "Type.h"
//#include "Generator.h"

using namespace std;

#define symmap std::unordered_map<std::string, Symbol*>

struct DataHeader{
    unsigned int size;
};

const int numTypes = 2;

enum SymbolType{
    Unknown = -1,
    SYM_VAR = 0,
    String = 1,
    SYM_FUNC = 2,
    SYM_BUILTINFUNC = 3,
    SYM_STRINGLIT = 4,
    SYM_CONTROL = 5,
    Procedure = 7
};

enum BuiltInFunction{
    F_PRINTNUM = 1,
    F_PRINT = 2,
    F_READINT = 3
};

enum ControlTypes{
    CIf = 1,
    CWhile = 2
};

class Symbol{
public:
    virtual SymbolType getType() = 0;
};

class Variable: public Symbol{
public:
    SymbolType getType();
    Variable(int a, Type t, int s);
    int getScope() { return scope; }
    int getAddr() { return addr; }
    int getSize() { return type.size(); }
    Type getVarType() { return type; }
    int getPtrAddr() { if(scope==0) return -addr; else return addr; }
private:
    int scope;
    int addr;
    Type type;
};

class Function: public Symbol{
public:
    SymbolType getType();
    Function();
    void setLabel(int l);
    void setVoid();
    void setRet(Type r);
    void addArg(Type t);
    int getLabel() { return label; }
    Type getRet() { return retVal; }
    bool retVoid() { return isVoid; }
    list<Type>* getArgs() { return &args; }
private:
    int label;
    bool isVoid;
    Type retVal;
    list<Type> args;
};

class BuiltInFunc: public Symbol{
public:
    SymbolType getType() { return SYM_BUILTINFUNC; }
    BuiltInFunction func;
    
    BuiltInFunc() {}
    ~BuiltInFunc() {}
};

class Scope{
public:
    Scope();
    ~Scope();
    int nextAddr();
    int size();
    void addVar(Type t) { vars.push_back(t); }
            
private:
    list<Type> vars;
};

//First scope is globals
class SymTab{
private:
    static std::ostream *sout;
    symmap tbl;
    
    list<Scope*> scopes;
    int scopeSize;
    
    int nextAddr(Type t);
    
public:
    int popLocals();

    static void setOutput(std::ostream *o) { sout = o; }
    SymTab();
    ~SymTab();
    
    void dump(std::ostream &out);
    
    Symbol* get(const std::string &key);
    bool keyExists(const std::string &key);
    
    
    
    Variable* addVar(const std::string &key, Type t);
    Function* addFunc(const std::string &key, Symbol *sym);
    void addBuiltInFunc(const std::string &key, BuiltInFunction fnc) {
        BuiltInFunc *f = new BuiltInFunc();
        f->func = fnc;
        
        tbl.insert({key, f});
    }
    
    void newScope();
    void popScope();
};

#endif
