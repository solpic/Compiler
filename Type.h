#ifndef TYPE_H
#define TYPE_H
#include <string>
#include "Tokenizer.h"
#include "Error.h"

typedef long t_int;
typedef double t_dbl;
typedef char t_char;
typedef int LBL;

enum PtrType{
	LOCAL = 0,
	GLOBAL = 1,
	MEM = 2
};

typedef long t_ptr;
    
#define LBL_SIZE sizeof(int)

#define INT_SIZE sizeof(t_int)
#define DBL_SIZE sizeof(t_dbl)
#define CHAR_SIZE sizeof(t_char)
#define PTR_SIZE sizeof(t_ptr)

enum Primitive{
    PRIM_INT = 0,
    PRIM_DBL = 1,
    PRIM_CHAR = 2
};

enum SerializedType{
    BIN_INT = 0,
    BIN_DBL = 1,
    BIN_CHAR = 2,
    BIN_PTR = 3
};

class StructDef{};

class Type{
public:
    Type();
    Type(Primitive p, int ptrLvl, StructDef *strct);
    int size();
    std::string toString() const;
    static std::string toString(SerializedType s);
    static bool equals(Type a, Type b);
    SerializedType toSerializedType();
    bool isBType(SerializedType t);
    
    static Type tChar() { return Type(PRIM_CHAR, 0, 0); }
    static Type tInt() { return Type(PRIM_INT, 0, 0); }
    static Type tDbl() { return Type(PRIM_DBL, 0, 0); }
    static Type charArray() { return Type(PRIM_CHAR, 1, 0); }
    
    friend std::ostream & operator << (std::ostream &out, const Type &t);
    static Type parse(Tokenizer *t);
    static Type parseNoPointer(Tokenizer *t);
    void parsePointerLevel(Tokenizer *t);
    static bool isType(Tokenizer *t);
    
    bool isChar() { return equals(*this, tChar()); }
    bool isInt() { return equals(*this, tInt()); }
    bool isDbl() { return equals(*this, tDbl()); }
    bool isPtr() { return pointerLevel>0; }
    
    void dereference();
    void reference() { pointerLevel++; }
    
    bool operator==(const Type &other);
    bool operator!=(const Type &other);
private:
    Primitive prim;
    int pointerLevel;
    StructDef *structSym;
};
    

#endif

