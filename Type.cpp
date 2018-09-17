#include "Type.h"
#include "SymTab.h"

using namespace std;

Type::Type()  {
    pointerLevel = 0;
    structSym = 0;
    prim = PRIM_INT;
}

SymTab* Type::symtab = 0;
void Type::setSymTab(SymTab *s) { symtab = s; }

Type::Type(Primitive p, int ptrLvl, StructDef *strct) {
    prim = p;
    pointerLevel = ptrLvl;
    structSym = strct;
}

int Type::size() const{
    if(pointerLevel>0)
        return PTR_SIZE;
    else if(structSym)
        return structSym->size();
    else if(prim==PRIM_INT)
        return INT_SIZE;
    else if(prim==PRIM_DBL)
        return DBL_SIZE;
    else if(prim==PRIM_CHAR)
        return CHAR_SIZE;
}

SerializedType Type::toSerializedType() {
    if(pointerLevel>0) return BIN_PTR;
    else return (SerializedType)prim;
}

string Type::toString() const{
    string s;
    if(structSym) {
        //Do something here
        s += structSym->name;
    }else{
        if(prim==PRIM_INT) s += "int";
        else if(prim==PRIM_DBL) s += "double";
        else if(prim==PRIM_CHAR) s += "char";
        else s += "ptr";
    }
    for(int i = 0; i<pointerLevel; i++) s += "*";
    return s;
}

string Type::toString(SerializedType s) {
    if(s==BIN_INT) return "int";
    else if(s==BIN_CHAR) return "char";
    else if(s==BIN_DBL) return "double";
    else return "ptr";
}

bool Type::isBType(SerializedType t) {
    return toSerializedType()==t;
}

bool Type::equals(Type a, Type b) {
    if(a.pointerLevel!=b.pointerLevel) return false;
    if(a.structSym!=b.structSym) return false;
    if(a.structSym&&(a.structSym==b.structSym)) return true;
    if(a.prim!=b.prim) return false;
    return true;
}

ostream & operator << (ostream &out, const Type &t) {
    out<<t.toString();
    return out;
}

bool Type::isType(Tokenizer *t) {
    if(t->cur()->equals("char")) return true;
    else if(t->cur()->equals("double")) return true;
    else if(t->cur()->equals("int")) return true;
    else if(symtab->keyExists(t->cur()->str())){
		if(symtab->get(t->cur()->str())->getType()==SYM_STRUCTDEF) return true;
	}
	return false;
}


Type Type::parseNoPointer(Tokenizer *t) {
    Primitive p;
    StructDef *s = 0;
    if(t->cur()->equals("char")) p = PRIM_CHAR;
    else if(t->cur()->equals("int")) p = PRIM_INT;
    else if(t->cur()->equals("double")) p = PRIM_DBL;
    else if(symtab->keyExists(t->cur()->str())) {
		Symbol *sym = symtab->get(t->cur()->str());
		if(sym->getType()==SYM_STRUCTDEF) {
			s = (StructDef*)sym; 
		}else error();
    }else error();
    t->advance();
    
    return Type(p, 0, s);
}

void Type::parsePointerLevel(Tokenizer *t) {
    pointerLevel = 0;
    while(t->cur()->equals("*")) {
        pointerLevel++;
        t->advance();
    }
}

Type Type::parse(Tokenizer *t) {
    Type type = parseNoPointer(t);
    type.parsePointerLevel(t);
    return type;
}

void Type::dereference() {
    if(pointerLevel>0) pointerLevel--;
    else{
        cout<<"Can't dereference non-pointer "<<*this<<endl;
        error();
    }
}


bool Type::operator==(const Type &other) {
    return equals(*this, other);
}

bool Type::operator!=(const Type &other) {
    return !equals(*this, other);
}
