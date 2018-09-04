#include "Parser.h"
#include "Error.h"

#include <sstream>

std::ostream* Parser::pout = &std::cout;

using namespace std;

const char *prefix = "Parser: ";

bool Parser::matchOperatorE(Operator &op) {
    if(testMatch("+")) {
        op = ADD;
        return true;
    }
    if(testMatch("-")) {
        op = SUBTRACT;
        return true;
    }
    if(testMatch("&&")) {
        op = AND;
        return true;
    }
    if(testMatch("||")) {
        op = OR;
        return true;
    }

    return false;
}

bool ptrIntOperatorException(Operator o, Type a, Type b) {
    return (o==ADD||o==SUBTRACT)
            &&(a.toSerializedType()==BIN_PTR&&b.toSerializedType()==BIN_INT);
}

void Parser::throwError() {
    cur()->tellPositionInformation(cout, t);
    error();
}

Type Parser::E() {
    Type a = T();
    Operator o;
    string opString = cur()->str();
    while(matchOperatorE(o)) {
        next();
        Type b = T();
        //Check matching types
        if(!ptrIntOperatorException(o, a, b)&&!Type::equals(a, b)) {
            cout<<"Expected appropriate operand type for type "<<a.toString()<<" and operator "<<opString<<" got "<<b.toString()<<" at ";
            throwError();
        }

        if(o==ADD) {
            g.addOp(new Add(a.toSerializedType()));
            asmOut += "add\n";
        } else if(o==SUBTRACT) {
            g.addOp(new Sub(a.toSerializedType()));
            asmOut += "sub\n";
        } else if(o==AND||o==OR) {
            //First check type
            if(a.isBType(BIN_CHAR)) {
                cout<<"Expected char got "<<a.toString()<<" at ";
                throwError();
            } else if(b.isBType(BIN_CHAR)) {
                cout<<"Expected char got "<<b.toString()<<" at ";
                throwError();
            }
            if(o==AND) {
                g.addOp(new And(a.toSerializedType()));
                asmOut += "and\n";
            } else if(o==OR) {
                g.addOp(new Or(a.toSerializedType()));
                asmOut += "or\n";
            }
            a = Type::tChar();
        }
    }
    return a;
}

bool Parser::matchOperatorT(Operator &op) {
    if(testMatch("*")) {
        op = MULT;
        return true;
    }
    if(testMatch("/")) {
        op = DIV;
        return true;
    }
    if(testMatch("==")) {
        op = EQ;
        return true;
    }
    if(testMatch("!=")) {
        op = NEQ;
        return true;
    }
    if(testMatch(">")) {
        op = GRTR;
        return true;
    }
    if(testMatch(">=")) {
        op = GRTR_EQ;
        return true;
    }
    if(testMatch("<")) {
        op = LESS;
        return true;
    }
    if(testMatch("<=")) {
        op = LESS_EQ;
        return true;
    }

    return false;
}

Type Parser::T() {
    Type a = F();
    Operator o;
    string opString = cur()->str();
    while(matchOperatorT(o)) {
        bool isBool = false;
        next();
        Type b = F();
        if(!Type::equals(a, b)) {
            cout<<"Expected "<<a<<" got "<<b<<" at ";
            throwError();
        }

        if(o==MULT) {
            g.addOp(new Mult(a.toSerializedType()));
            asmOut += "mult\n";
        } else if(o==DIV) {
            g.addOp(new Div(a.toSerializedType()));
            asmOut+="div\n";
        } else if(o==EQ) {
            g.addOp(new CmpEq(a.toSerializedType()));
            isBool = true;
            asmOut+="eq\n";
        } else if(o==NEQ) {
            g.addOp(new CmpNeq(a.toSerializedType()));
            isBool = true;
            asmOut+= "neq\n";
        } else if(o==GRTR) {
            g.addOp(new CmpGrtr(a.toSerializedType()));
            isBool = true;
            asmOut += "grtr\n";
        } else if(o==GRTR_EQ) {
            g.addOp(new CmpGrtrEq(a.toSerializedType()));
            isBool = true;
            asmOut += "grtr_eq\n";
        } else if(o==LESS) {
            g.addOp(new CmpLess(a.toSerializedType()));
            isBool = true;
            asmOut += "less\n";
        } else if(o==LESS_EQ) {
            g.addOp(new CmpLessEq(a.toSerializedType()));
            isBool = true;
            asmOut += "less_eq\n";
        }
        if(isBool) {
            a = Type::tChar();
        }
    }
    return a;
}

Type Parser::typecast() {
    Type to = Type::parse(t);

    match("(");
    next();
    Type from = E();
    match(")");
    next();

    SerializedType to_actual = to.toSerializedType();
    SerializedType frm_actual = from.toSerializedType();

    if(to_actual!=frm_actual) {
        g.addOp(new Typecast(to_actual, frm_actual));
        asmOut += "typecast "+to.toString()+" to "+from.toString();+"\n";
    }else{
        cout<<"Empty typecast from "<<from<<" to "<<to<<endl;
        throwError();
    }

    return to;
}

void Parser::function(Symbol &s) {
    string fname = cur()->str();
    //Push return vals onto stack
    //Push arguments onto stack
    //Push variables onto stack
    //Push function label onto stack
    //Call
    next();

    if(s.retsScopeSize>0) {
        char returns[s.retsScopeSize];
        g.addOp(new PushI(s.retsScopeSize, returns));
        asmOut += "pushempty "+to_string(s.retsScopeSize)+"\n";
    }
    match("(");
    next();
    for(int i = 0; i<s.numArg; i++) {
        E();
        if(i<s.numArg-1) {
            match(",");
            next();
        }
    }
    match(")");
    next();

    cout<<"Locals: "<<s.localsScopeSize<<endl;
    if(s.localsScopeSize>0) {
        char buffer[s.localsScopeSize];
        g.addOp(new PushI(s.localsScopeSize, buffer));
        asmOut += "pushempty "+to_string(s.localsScopeSize)+"\n";
    }

    g.addOp(new PushLbl(s.funcLabel));
    asmOut += "push "+fname+"\n";

    g.addOp(new Call());
    asmOut += "call\n";
}

Type Parser::F() {
    //NUM | (E) | VARIABLE | &VARIABLE | STRING_LIT | *E | typecast(E) | true | false | !F | -F
    if(testMatch("!")) {
        *pout<<prefix<<"!"<<endl;
        next();
        if(testMatch("!")) {
            cout<<"! followed by ! is no operation at ";
            throwError();
        }
        Type o = F();
        if(o.isChar()) {
            cout<<"Expected char for ! operator, got "<<o<<" at ";
            throwError();
        }

        g.addOp(new Not());
        asmOut+="not\n";
        return Type::tChar();
    } else if(testMatch("-")) {
        *pout<<prefix<<"-"<<endl;
        next();
        if(testMatch("-")) {
            cout<<"- followed by - is no operation at ";
            throwError();
        }
        Type o = F();
        if(o.isInt()||o.isDbl()) {
            cout<<"Expected int or double for - operator, got "<<o<<" at ";
            throwError();
        }

        g.addOp(new Negative(o.toSerializedType()));
        asmOut+="negative\n";
        return o;
    } else if(testMatch("true")||testMatch("false")) {
        *pout<<prefix<<"TRUE/FALSE literal"<<endl;
        t_char c = testMatch("true")?1:0;
        g.addOp(new PushI(sizeof(c), &c));
        asmOut += "pushi "+to_string(c)+"\n";
        next();
        return Type::tChar();
    } else if(testMatchType(TK_INT)) {
        *pout<<prefix<<"Matched number literal "<<cur()->str()<<endl;
        t_int i = stoi(cur()->str());

        g.addOp(new PushI(sizeof(i), &i));
        asmOut += "pushi "+cur()->str()+", size: "+to_string(sizeof(i))+"\n";

        next();

        return Type::tInt();
    } else if(testMatchType(TK_DOUBLE)) {
        *pout<<prefix<<"Matched number literal "<<cur()->str()<<endl;
        t_dbl d = stod(cur()->str());

        g.addOp(new PushI(sizeof(d), &d));
        asmOut += "pushi "+cur()->str()+", size: "+to_string(sizeof(d))+"\n";

        next();

        return Type::tDbl();
    } else if(testMatchType(TK_CHAR)) {
        *pout<<prefix<<"Matched character literal "<<cur()->str()<<endl;
        t_char c = cur()->str()[0];

        g.addOp(new PushI(sizeof(c), &c));
        asmOut += "pushi "+cur()->str()+", size: "+to_string(sizeof(c))+"\n";

        next();

        return Type::tChar();
    } else if(testMatch("(")) {
        match("(");
        next();
        Type v = E();
        match(")");
        next();
        return v;
    } else if(testMatch("&")) {
        //Get address of var
        match("&");
        next();
        Symbol s = sym.get(cur()->str());
        if(s.type==SYM_VAR) {
            g.addOp(new PushI(PTR_SIZE, &s.addr));
            asmOut += "pushI &"+cur()->str()+": "+to_string(s.addr)+"\n";
            next();
            return s.varType;
        } else {
            cout<<"Expected variable, got "<<cur()->str()<<" at ";
            throwError();
        }
    } else if(testMatch("*")) {
        //Dereference pointer
        match("*");
        next();
        Type p = E();
        if(p.isPtr()) {
            cout<<"Expected pointer for dereference, instead got "<<p<<endl;
            error();
        }

        g.addOp(new PushPtr(p.size()));
        asmOut += "pushptr "+p.toString()+": "+to_string(p.size())+"\n";

        p.dereference();
        return p;
    } else if(testMatchType(TK_IDEN)) {
        //First check if its a typecast
        if(testMatch("int")||testMatch("double")||testMatch("char")) {
            return typecast();
        }
        //Is it variable or function

        Symbol s = sym.get(cur()->str());
        if(s.type==SYM_VAR) {
            g.addOp(new Push(s.addr, s.size()));
            asmOut += "push "+cur()->str()+":"+to_string(s.addr)+", size:"+to_string(s.size())+"\n";
            next();


            return s.varType;
        } else if(s.type==SYM_FUNC) {
			function(s);
            return s.returnType;
        } else {
            cout<<"Expected variable or function, got "<<cur()->str()<<" at ";
            throwError();
        }
    } else {
        cout<<"Unexpected token "<<cur()->str()<<" at ";
        throwError();
    }
}

void Parser::compile(const char* fname) {
    *pout<<prefix<<"Data size: "<<sym.getSize()<<endl;
    sym.dump(*pout);
    g.resolveLabels(sym.getSize());
    g.generate(fname, sym.getSize(), sym);
}

Token* Parser::cur() {
    return t->cur();
}

Token* Parser::next() {
    return t->advance();
}

void Parser::match(const char *s) {
    if(!cur()->equals(s)) {
        cout<<"Expected token "<<s<<", got "<<cur()->str()<<" at ";
        throwError();
    } else {
        *pout<<prefix<<"Matched "<<s<<endl;
    }
}

void Parser::matchType(TokenType type, const char *expected) {
    if(cur()->getType()!=type) {
        cout<<"Expected type "<<type<<", got "<<cur()<<" at ";
        throwError();
    } else {
        *pout<<prefix<<"Matched type of "<<expected<<endl;
    }
}

bool Parser::testMatchType(TokenType type) {
    return cur()->getType()==type;
}

bool Parser::testMatch(const char *s) {
    return cur()->equals(s);
}

void Parser::assignment() {
    string varName = cur()->str();
    *pout<<prefix<<"Assignment of "<<varName<<endl;
    next();
    match("=");
    next();

    Type v = E();
    Symbol s = sym.get(varName);

    Type q = s.varType;

    if(Type::equals(v, q)) {
        cout<<"Can't assign "<<v<<" to "<<q<<" at ";
        throwError();
    }

    g.addOp(new Pop(s.getAddr(), s.size()));
    asmOut+="pop "+varName+":"+to_string(s.getAddr())+"\n";
}

void Parser::pointerAssignment() {
    Type v = E();
    match("=");
    next();
    Type q = E();
    v.dereference();
    if(v!=q) {
        cout<<"Can't dereference "<<q<<" into "<<v<<" at ";
        throwError();
    }

    g.addOp(new PopToPtr(q.size()));
    asmOut+="poptoptr "+to_string(q.size())+"\n";
}

void Parser::controlStatement(Symbol &s) {
    /*
    if(s.cType==CIf) {
    //If statement
    next();
    E();
    match(":"); next();

    int label = g.curLabel++;
    g.addOp(Op::pushLbl(label));
    asmOut+="pushlbl L"+to_string(label)+"\n";
    g.addOp(Op::ifnotjump());
    asmOut+="ifnjmp\n";

    statements();
    match("end"); next();

    g.addOp(Op::label(label));
    asmOut+="L"+to_string(label)+":\n";
    }else if(s.cType==CWhile) {
    //While statement
    next();

    int labelStart = g.curLabel++;
    g.addOp(Op::label(labelStart));
    asmOut+="L"+to_string(labelStart)+":\n";

    E();
    match(":"); next();
    int labelEnd = g.curLabel++;

    g.addOp(Op::pushLbl(labelEnd));
    asmOut+="pushlbl L"+to_string(labelEnd)+"\n";
    g.addOp(Op::ifnotjump());
    asmOut+="ifnjmp\n";

    statements();
    match("end"); next();

    g.addOp(Op::pushLbl(labelStart));
    asmOut+="pushlbl L"+to_string(labelStart)+"\n";
    g.addOp(Op::opJmp());
    asmOut+="jmp\n";

    g.addOp(Op::label(labelEnd));
    asmOut+="L"+to_string(labelEnd)+":\n";

    }
    */
}

void Parser::builtInFunction(Symbol &s) {
    if(s.builtInFunc==F_PRINTNUM) {
        next();
        match("(");
        next();
        Type v = E();
        match(")");
        next();

        g.addOp(new PrintNum(v.toSerializedType()));
        asmOut+="printnum "+v.toString()+"\n";
    }
}

void Parser::statements() {
    *pout<<prefix<<"Current statement: "<<cur()->str()<<endl;
    if(sym.keyExists(cur()->str())) {
        Symbol s = sym.get(cur()->str());
        SymbolType type = s.getType();
        if(type==SYM_VAR) {
            //Must be assignment
            assignment();
            match(";");
            next();
        } else if(type==SYM_BUILTINFUNC) {
            builtInFunction(s);
            match(";");
            next();
        } else if(type==SYM_CONTROL) {
            controlStatement(s);
        } else if(type==SYM_FUNC) {
            function(s);
            match(";");
            next();
        }
    } else if(testMatch("*")) {
        //Pointer assignment
        next();
        pointerAssignment();
        match(";");
        next();
    } else if(testMatch("return")) {
        next();
        if(!currentFunc) {
            cout<<"Can't return without enclosing function at ";
            throwError();
        }
        //Evaluate return arguments and push them into old stack
        //for(int i = 0; i<currentFunc->numRet; i++) {
        if(!currentFunc->isVoid) {
            int i = 0;
            E();
            Symbol ret = sym.get(to_string(i));

            g.addOp(new Pop(ret.addr, ret.size()));
            asmOut += "pop return value "+to_string(i)+"\n";
        }
        match(";");
        next();
        //Return statement
        //We need to pop all the stuff off the stack
        int scope = currentFunc->localsScopeSize + currentFunc->argsScopeSize;
        g.addOp(new PushI(LBL_SIZE, &scope));
        asmOut += "pushi:fnc_scope "+to_string(scope)+", size: "+to_string(LBL_SIZE)+"\n";

        g.addOp(new Return());
        asmOut += "return\n";
    }else {
        cout<<"Expected statement, got "<<cur()->str()<<" at ";
        cur()->tellPositionInformation(cout, t);
        sym.dump(cout);
        error();
    }
}

Parser::Parser(Tokenizer *tok) {
    t = tok;
    currentFunc = 0;

    Symbol s(SYM_BUILTINFUNC);

    s.builtInFunc = F_PRINTNUM;
    sym.addSymbol("printvar", s);

    Symbol s2(SYM_BUILTINFUNC);
    s2.builtInFunc = F_PRINT;
    sym.addSymbol("print", s2);

    Symbol sIf(SYM_CONTROL);
    sIf.cType = CIf;
    sym.addSymbol("if", sIf);

    Symbol sWhile(SYM_CONTROL);
    sWhile.cType = CWhile;
    sym.addSymbol("while", sWhile);

    Symbol sReadInt(SYM_BUILTINFUNC);
    sReadInt.builtInFunc = F_READINT;
    sym.addSymbol("readint", sReadInt);
}

void Parser::func_def() {
    next();

    match(":");
    next();
    //We should check that return values and args match up
    //But we won't
    while(testMatch("void")||testMatch("int")||testMatch("double")||testMatch("char")) {
        next();
        if(testMatch(",")) next();
    }

    //func name
    if(!testMatchType(TK_IDEN)) {
        cout<<"Expected identifier, got "<<cur()->str()<<" at ";
        throwError();
    }

    //Check that it exists
    if(!sym.keyExists(cur()->str())) {
        cout<<"Trying to define undeclared function "<<cur()->str()<<" at ";
        throwError();
    }

    Symbol *s = sym.getPtr(cur()->str());
    next();
    currentFunc = s;
    match("(");
    next();

    vector<string> argNames;
    vector<int> pointerLevels;
    pointerLevels.resize(s->numArg);
    argNames.resize(s->numArg);
    int i = 0;
    //We should check that args match but we won't
    while(testMatch("int")||testMatch("double")||testMatch("char")) {
        next();
        pointerLevels[i] = 0;
        while(testMatch("*")) {
            pointerLevels[i]++;
            next();
        }
        if(!testMatchType(TK_IDEN)) {
            cout<<"Expected identifier, got "<<cur()->str()<<" at ";
            throwError();
        }
        if(sym.keyExists(cur()->str())) {
            cout<<"Duplicate symbol "<<cur()->str()<<" at ";
            throwError();
        }
        argNames[i++] = cur()->str();
        next();
        if(testMatch(",")) next();
    }
    match(")");
    next();
    match("{");
    next();

    //Add label
    g.addOp(new Label(s->funcLabel));
    asmOut += "L"+to_string(s->funcLabel)+":\n";

    //Args, vars, return location
    //First arg has addr 0
    //Return location has addr scopeSize
    //Return location is top of stack
    /*
     * 3 byte
     * BYTE | BYTE | BYTE | RETURN |
     * scopesize = 3
     * We need to subtract scopeSize from addresses for proper values
     * local[0-lbl_size] = return
     * local[-lbl_size-var_size] = next
     */
    //Subtract labelsize +scopesize from each addr
    //Variable declarations
    sym.funcScope = true;
    sym.scopeSize = 0;

    //for(i = 0; i<s->numRet; i++) {
    Symbol ret(SYM_VAR);
    ret.varType = s->returnType;

    sym.addSymbol("0", ret);
    //}

    int retScopeSize = sym.scopeSize;

    for(i = 0; i<s->numArg; i++) {
        Symbol arg(SYM_VAR);
        arg.varType = s->args[i];

        cout<<"Local "<<argNames[i]<<" p "<<pointerLevels[i]<<endl;

        sym.addSymbol(argNames[i], arg);
    }

    int argScopeSize = sym.scopeSize - retScopeSize;
    declarations(false);
    //We use these later
    s->localsScopeSize = sym.scopeSize - argScopeSize - retScopeSize;
    s->argsScopeSize = argScopeSize;
    s->retsScopeSize = retScopeSize;

    //Fix addresses
    sym.fixLocalAddresses();
    while(!testMatch("}")) statements();
    match("}");
    next();
    match(";");
    next();

    //Delete locals
    sym.deleteLocals();
    sym.funcScope = false;

    currentFunc = 0;
}

void Parser::parse() {

    labelMain = g.curLabel++;
    g.addOp(new PushLbl(labelMain));
    asmOut+="pushlbl L"+to_string(labelMain)+"\n";
    g.addOp(new Jump());
    asmOut+="jmp\n";

    declarations(true);
    while(testMatch("function")) func_decl();
    while(testMatch("define")) func_def();
    cout<<"Done with func defs"<<endl;

    g.addOp(new Label(labelMain));
    asmOut += "L"+to_string(labelMain)+":\n";
    while(!testMatch("EOF")) statements();
    match("EOF");
    next();

    g.addOp(new Halt());
    asmOut += "halt\n";
}

//<var_decl> -> <type_name> <var_decl_elt>;
//<var_decl_elt> -> ** <var_name> <var_decl_elt_tail>
//<var_decl_elt_tail>, <var_decl_elt> | E
void Parser::var_decl() {
    Type t = Type::parseNoPointer(t);
    *pout<<prefix<<"Variable type is "<<t<<endl;

    list<string> varNames;
    list<int> pointerLevels;
    bool listDone = false;
    do {
        //Match asterisks for pointer
        int pointerLevel = 0;
        while(testMatch("*")) {
            next();
            pointerLevel++;
        }

        matchType(TK_IDEN, "Identifier");
        //Check that variable isn't in symbol table or the list
        if(sym.keyExists(cur()->str())
                ||find(varNames.begin(), varNames.end(), cur()->str())!=varNames.end()) {
            cout<<"Duplicate symbol "<<cur()->str()<<" at ";
            throwError();
        }

        varNames.push_back(cur()->str());
        pointerLevels.push_back(pointerLevel);

        *pout<<prefix<<"Variable: "<<cur()->str()<<endl;
        next();
        if(testMatch(";")) {
            listDone = true;
            next();
        } else if(testMatch(",")) {
            next();
        } else {
            cout<<"Expected ; or , got "<<cur()->str()<<" at ";
            throwError();
        }
    } while(!listDone);

    //Now we actually add the variables
    list<string>::iterator i = varNames.begin();
    list<int>::iterator j = pointerLevels.begin();
    while(i!=varNames.end()) {
        Symbol s(SYM_VAR);
        s.varType = varType;

        sym.addSymbol(*i, s);

        i++;
        j++;
    }
}

void Parser::func_decl() {
	next();
    match(":");
    next();
    
    Type ret;
    bool isVoid = false;
    if(testMatch("void")) {
        isVoid = true;
        next();
    } else {
        ret = Type::parse(t);
    }

    //func name
    if(!testMatchType(TK_IDEN)) {
        cout<<"Expected identifier, got "<<cur()->str()<<" at ";
        throwError();
    }

    //Check key is unique
    if(sym.keyExists(cur()->str())) {
        cout<<"Key "<<cur()->str()<<" already exists ";
        throwError();
    }

    string funcName = cur()->str();
    next();

    match("(");
    next();
    //Match arguments
    list<Type> args;
    bool done = false;
    if(testMatch(")")) {
        done = true;
        next();
    }
    while(!done) {
        args.push_back(Type::parse(t));
        /*
        Variable v;
        if(testMatch("int")) {
        	v.type = VAR_INT;
        	args.push_back(v);
        }else if(testMatch("double")) {
        	v.type = VAR_DOUBLE;
        	args.push_back(v);
        }else if(testMatch("char")) {
        	v.type = VAR_CHAR;
        	args.push_back(v);
        }*/
        //next();
        //This allows for trailing comma
        if(testMatch(",")) next();
        else if(testMatch(")")) {
            next();
            done = true;
        } else {
            cout<<"Expected , or ) in function declaration, got "<<cur()->str()<<" at ";
            throwError();
        }
    }
    match(";");
    next();

    Symbol s(SYM_FUNC);
    s.isVoid = isVoid;
    s.returnType = ret;

    /*s.numRet = retVals.size();

    s.rets = new Variable[retVals.size()];
    int j = 0;
    for(auto i: retVals) {
    s.rets[j++] = i;
    }*/

    s.numArg = args.size();
    s.args = new Type[args.size()];
    int j = 0;
    for(auto i: args) s.args[j++] = i;

    s.funcLabel = g.curLabel++;
    sym.addSymbol(funcName, s);
}

void Parser::declarations(bool funcs) {
    //Different types of declarations
    bool declarationsDone = false;
    while(!declarationsDone) {
        if(Type::isType(t)) {
            var_decl();
        }else if(funcs&&testMatch("function")) {
            //Function declaration
            func_decl();
        } else {
            declarationsDone = true;
        }
    }
}
