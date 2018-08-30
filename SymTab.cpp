#include "SymTab.h"

std::ostream* SymTab::sout = &std::cout;

int varSizes[] = {INT_SIZE, DBL_SIZE, CHAR_SIZE};

VarType parseToVar(ParseVar p) {
    if(p.pointerLevel>0) return VAR_PTR;
    else return p.v;
}

using namespace std;

const char *typeStrings[] = {"int", "string", "function", "built_in_function", "string literal", "control statement", "label", "procedure"};

SymbolType Symbol::stringToType(const std::string &s) {
    for(int i = 0; i<numTypes; i++) {
        if(!s.compare(typeStrings[i])) return (SymbolType)i;
    }
    
    return Unknown;
}

const char *varTypeStrings[] = {"int", "double", "char", "pointer", "void"};

const char *typeToString(VarType v) {
    return varTypeStrings[(int)v];
}


Symbol SymTab::get(const std::string &key) {
    return tbl.at(key);
}

Symbol* SymTab::getPtr(const std::string &key) {
    return &tbl.find(key)->second;
}


SymTab::SymTab() {
    size = 0;
    curString = 0;
    funcScope = false;
}

SymTab::~SymTab() {
    
}

bool SymTab::keyExists(const std::string &key) {
    return tbl.find(key)!=tbl.end();
}

void SymTab::dump(ostream &out) {
    out<<"Symbol Table Dump: "<<endl;
    for(auto i: tbl) {
        out<<i.first<<endl;
        out<<"\t"<<i.second.size()<<endl;
    }
}

void SymTab::fillStringLiterals(char *data) {
    for(auto i: tbl) {
        if(i.second.getType()==SYM_STRINGLIT) {
            memcpy(data+i.second.getAddr(), i.second.lit, strlen(i.second.lit));
            data[i.second.getAddr()+strlen(i.second.lit)] = 0;
            delete[] i.second.lit;
        }
    }
}

void SymTab::deleteLocals() {
    for(auto it = tbl.begin(); it!=tbl.end(); it++) {
        if(it->second.local) {
            auto tmp = it++;
            tbl.erase(tmp);
        }
    }
}

void SymTab::fixLocalAddresses() {
    for(auto it = tbl.begin(); it!=tbl.end(); it++) {
        if(it->second.local) {
            it->second.addr -= LBL_SIZE+LBL_SIZE + scopeSize;
        }
    }
}

void SymTab::addSymbol(const std::string &key, Symbol sym) {
    if(sym.getType()==SYM_VAR) {
        if(!funcScope) {
            sym.setAddr(size+sizeof(DataHeader));
            size+=sym.size();
        }else{
            sym.setAddr(scopeSize);
            scopeSize += sym.size();
            sym.local = true;
        }
    }else if(sym.getType()==SYM_STRINGLIT) {
        sym.setAddr(size+sizeof(DataHeader));
        size += strlen(sym.lit) + 1;
    }
    
    tbl.insert({key, sym});
    *sout<<"Added symbol "<<key<<" of type "<<typeStrings[sym.getType()]<<endl;
}