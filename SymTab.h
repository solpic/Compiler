#ifndef SYMTAB_H
#define SYMTAB_H

#include <unordered_map>
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
//#include "Generator.h"

#define symmap std::unordered_map<std::string, Symbol>

typedef long t_int;
typedef double t_dbl;
typedef char t_char;
typedef int LBL;
typedef int t_ptr;
#define LBL_SIZE sizeof(int)

#define INT_SIZE sizeof(t_int)
#define DBL_SIZE sizeof(t_dbl)
#define CHAR_SIZE sizeof(t_char)
#define PTR_SIZE sizeof(t_ptr)
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

extern int varSizes[];

enum VarType{
    VAR_INT = 0,
    VAR_DOUBLE = 1,
    VAR_CHAR = 2,
    VAR_PTR = 3, 
    VAR_VOID = 4
};


struct ParseVar{
    VarType v;
    int pointerLevel;
};

VarType parseToVar(ParseVar p);

struct Variable{
    VarType type;
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
    VarType varType;
    /*
     * 1 is int *j;
     * 2 is int **j;
     * etc
     */
    VarType pointerType;
    int pointerLevel;
    
    //If type==SYM_FUNC
    //int numRet;
    //Variable *rets;
    
    bool isVoid;
    ParseVar returnType;
    
    int numArg;
    ParseVar *args;
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
            if(pointerLevel>0) return PTR_SIZE;
            switch(varType) {
                case VAR_INT:
                return INT_SIZE;
                break;
                case VAR_DOUBLE:
                return DBL_SIZE;
                break;
                case VAR_CHAR:
                return CHAR_SIZE;
                default:
                return 0;
                break;
            }
        }
    
    }
    SymbolType getType() { return type; }
    ~Symbol() {}
};
const char *typeToString(VarType v);

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
