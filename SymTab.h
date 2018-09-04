#ifndef SYMTAB_H
#define SYMTAB_H

#include <unordered_map>
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "Type.h"
//#include "Generator.h"

#define symmap std::unordered_map<std::string, Symbol>

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
    bool local;
    SymbolType type;
    
    //If type==SYM_BUILTINFUNC
    BuiltInFunction builtInFunc;
    
    //If type==SYM_VAR
    Type varType;
    /*
     * 1 is int *j;
     * 2 is int **j;
     * etc
     */
    
    //If type==SYM_FUNC
    //int numRet;
    //Variable *rets;
    
    bool isVoid;
    Type returnType;
    
    int numArg;
    Type *args;
    LBL funcLabel;
    int localsScopeSize;
    int argsScopeSize;
    int retsScopeSize;
    
    int addr;
    ControlTypes cType;
    char *lit;
    static SymbolType stringToType(const std::string &s);
    Symbol(SymbolType t) {
        type = t;
        local = false;
    }
    void setAddr(int a) { addr = a; }
    int getAddr() { return addr; }

    int size() {
        if(type==SYM_VAR) {
            return varType.size();
        }
    
    }
    SymbolType getType() { return type; }
    ~Symbol() {}
};

class SymTab{
private:
    static std::ostream *sout;
    symmap tbl;
    int size;
public:
    bool funcScope;
    int scopeSize;
    static void setOutput(std::ostream *o) { sout = o; }
    int curString;
    void fillStringLiterals(char *data);
    SymTab();
    ~SymTab();
    
    void dump(std::ostream &out);
    
    Symbol get(const std::string &key);
    Symbol* getPtr(const std::string &key);
    bool keyExists(const std::string &key);
    void addSymbol(const std::string &key, Symbol sym);
    void deleteLocals();
    void fixLocalAddresses();
    int getSize() { return size; }
};

#endif
