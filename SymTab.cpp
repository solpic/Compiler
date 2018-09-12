#include "SymTab.h"

std::ostream* SymTab::sout = &std::cout;

using namespace std;

const char *typeStrings[] = {"int", "string", "function", "built_in_function", "string literal", "control statement", "label", "procedure"};

SymbolType Variable::getType() {
    return SYM_VAR;
}

Variable::Variable(int a, Type t, int s) {
    addr = a;
    type = t;
    scope = s;
}

SymbolType Function::getType() {
    return SYM_FUNC;
}

Function::Function() {
    label = -1;
    isVoid = false;
}

void Function::setLabel(int l) {
    label = l;
}

void Function::setVoid() {
    isVoid = true;
}

void Function::setRet(Type r) {
    retVal = r;
}

void Function::addArg(Type t) {
    args.push_back(t);
}

Symbol* SymTab::get(const std::string &key) {
    return tbl.at(key);
}


SymTab::SymTab() {
    scopeSize = 0;
    
    scopes.push_back(new Scope());
}

SymTab::~SymTab() {
    delete scopes.front();
}

bool SymTab::keyExists(const std::string &key) {
    return tbl.find(key)!=tbl.end();
}

void SymTab::dump(ostream &out) {
    out<<"Symbol Table Dump: "<<endl;
    for(auto i: tbl) {
        out<<i.first<<endl;
    }
    //for(auto i: tbl) {
    //    out<<i.first<<endl;
     //   out<<"\t"<<i.second.size()<<endl;
    //}
}


Scope::Scope() {
}

Scope::~Scope(){}

int Scope::size() {
    int s = 0;
    for(auto v: vars) s += v.size();
    
    return s;
}

void SymTab::newScope() {
    //We don't want the global scope added here
    if(scopes.size()>1) {scopeSize += scopes.back()->size();}
    scopes.push_back(new Scope());
}

void SymTab::popScope() {
    int s = scopes.size();
    
    auto sym = tbl.begin();
    while(sym!=tbl.end()) {
		Variable *v = (Variable*)sym->second;
        if(sym->second->getType()==SYM_VAR&&v->getScope()==s) {
            Symbol *tmp = sym->second;
            sym = tbl.erase(sym);
            
            delete tmp;
        }else sym++;
    }
    
    Scope *scp = scopes.back();
    scopes.pop_back();
    delete scp;
}


//Gets total size of locals scope for returning
int SymTab::popLocals() {
    return scopeSize;
}

int SymTab::nextAddr(Type t) {
    int a = scopeSize + scopes.back()->size();
    scopes.back()->addVar(t);
    
    if(scopes.size()==1) a = -a;
    
    return a;
}

Variable* SymTab::addVar(const std::string &key, Type t) {
    Variable *v = new Variable(nextAddr(t), t, scopes.size());
    tbl.insert({key, v});
    return v;
}

Function* SymTab::addFunc(const std::string &key, Symbol *sym) {
    tbl.insert({key, sym});
    return (Function*)sym;
}
