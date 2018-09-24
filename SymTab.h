#ifndef SYMTAB_H
#define SYMTAB_H

#include <unordered_map>
#include <string>
#include <list>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "Type.h"
#include "Generator.h"

using namespace std;

#define symmap std::unordered_map<std::string, Symbol*>

struct DataHeader{
    unsigned int size;
};



enum HookCode{
	HOOK_MALLOC = 1,
	HOOK_DELETE = 2
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
    SYM_STRUCTDEF = 6,
    Procedure = 7,
    SYM_HOOK = 8
};

enum BuiltInFunction{
    F_PRINTNUM = 1,
    F_PRINT = 2,
    F_READINT = 3,
    F_HOOK = 4
};

enum ControlType{
    C_IF = 1,
    C_WHILE = 2
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
private:
    int scope;
    int addr;
    Type type;
};

class StructDef: public Symbol{
public:
	StructDef() {}
	~StructDef() {}
	SymbolType getType() { return SYM_STRUCTDEF; }
	
	list<Type> vars;
	list<string> varNames;
	
	string name;
	
	
    friend std::ostream & operator << (std::ostream &out, const StructDef &t);
	
	int offset(string &name) {
		int o = 0;
		auto v = vars.begin();
		auto n = varNames.begin();
		while(name!=*n) { 
			o += (*v).size();
			v++; n++;
		}
		
		return o;
	}
	
	int size(string &name) {
		auto v = vars.begin();
		auto n = varNames.begin();
		while(name!=*n) {v++; n++;}
		return (*v).size();
	}
	
	Type type(string &name) {
		auto v = vars.begin();
		auto n = varNames.begin();
		while(name!=*n) {v++; n++;}
		return *v;
	}
	
	int size() {
		int s = 0;
		for(auto t: vars) s += t.size();
		return s;
	}
};

class StringLit: public Symbol{
public:
	SymbolType getType() { return SYM_STRINGLIT; }
	StringLit(int o, string s) {
		offset = o;
		value = s;
	}
	int getOffset() { return offset; }
	int size() { return value.size(); }
	const char *c_str() { return value.c_str(); }
private:
	string value;
	int offset;
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
    
    bool isHook;
    HookCode hookCode;
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

class ControlStatement: public Symbol{
public:
	SymbolType getType() { return SYM_CONTROL; }
	ControlType type;
	
	ControlStatement() {}
	~ControlStatement() {}
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
	int stringOffset;
    static std::ostream *sout;
    symmap tbl;
    
    list<Scope*> scopes;
    list<StringLit*> strings;
    int scopeSize;
    
    int nextAddr(Type t);
    
public:
	int stringDataSize() { return stringOffset; }
	void writeStringData(char *data);
    int popLocals();

    static void setOutput(std::ostream *o) { sout = o; }
    SymTab();
    ~SymTab();
    
    void dump(std::ostream &out);
    
    Symbol* get(const std::string &key);
    bool keyExists(const std::string &key);
    
    
    void addStruct(const std::string &key, StructDef *s) {
		s->name = key;
		tbl.insert({key, s});
	}
    Variable* addVar(const std::string &key, Type t);
    Function* addFunc(const std::string &key, Symbol *sym);
    void addBuiltInFunc(const std::string &key, BuiltInFunction fnc) {
        BuiltInFunc *f = new BuiltInFunc();
        f->func = fnc;
        
        tbl.insert({key, f});
    }
    StringLit* addStringLit(std::string value);
    void addControlStatement(const std::string &key, ControlType cntrl) {
		ControlStatement *c = new ControlStatement();
		c->type = cntrl;
		
		tbl.insert({key, c});
	}
		
    
    void newScope();
    int popScope();
};

#endif
