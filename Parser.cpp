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
            g.addOp(new Add(a.toSerializedType()), "add");
        } else if(o==SUBTRACT) {
            g.addOp(new Sub(a.toSerializedType()), "sub");
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
                g.addOp(new And(a.toSerializedType()), "and");
            } else if(o==OR) {
                g.addOp(new Or(a.toSerializedType()), "or");
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
            g.addOp(new Mult(a.toSerializedType()), "mult");
        } else if(o==DIV) {
            g.addOp(new Div(a.toSerializedType()), "div");
        } else if(o==EQ) {
            g.addOp(new CmpEq(a.toSerializedType()), "eq");
            isBool = true;
        } else if(o==NEQ) {
            g.addOp(new CmpNeq(a.toSerializedType()), "neq");
            isBool = true;
        } else if(o==GRTR) {
            g.addOp(new CmpGrtr(a.toSerializedType()), "grtr");
            isBool = true;
        } else if(o==GRTR_EQ) {
            g.addOp(new CmpGrtrEq(a.toSerializedType()), "grtr_eq");
            isBool = true;
        } else if(o==LESS) {
            g.addOp(new CmpLess(a.toSerializedType()), "less");
            isBool = true;
        } else if(o==LESS_EQ) {
            g.addOp(new CmpLessEq(a.toSerializedType()), "less_eq");
            isBool = true;
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
        g.addOp(new Typecast(to_actual, frm_actual), "typecast "+to.toString()+" to "+from.toString());
    }else{
        cout<<"Empty typecast from "<<from<<" to "<<to<<endl;
        throwError();
    }

    return to;
}

void Parser::function(Function *f) {
    string fname = cur()->str();
    next();


    match("(");
    next();
    
    int numArg = f->getArgs()->size();
    
    for(int i = 0; i<numArg; i++) {
        E();
        if(i<numArg-1) {
            match(",");
            next();
        }
    }
    match(")");
    next();

    g.addOp(new PushLbl(f->getLabel()), "push "+fname);

    g.addOp(new Call(), "call");
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

        g.addOp(new Not(), "not");
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

        g.addOp(new Negative(o.toSerializedType()), "negative");
        return o;
    } else if(testMatch("true")||testMatch("false")) {
        *pout<<prefix<<"TRUE/FALSE literal"<<endl;
        t_char c = testMatch("true")?1:0;
        g.addOp(new PushI(sizeof(c), &c), "pushi "+to_string(c));
        next();
        return Type::tChar();
    } else if(testMatchType(TK_INT)) {
        *pout<<prefix<<"Matched number literal "<<cur()->str()<<endl;
        t_int i = stoi(cur()->str());

        g.addOp(new PushI(sizeof(i), &i), "pushi "+cur()->str()+", size: "+to_string(sizeof(i)));

        next();

        return Type::tInt();
    } else if(testMatchType(TK_DOUBLE)) {
        *pout<<prefix<<"Matched number literal "<<cur()->str()<<endl;
        t_dbl d = stod(cur()->str());

        g.addOp(new PushI(sizeof(d), &d), "pushi "+cur()->str()+", size: "+to_string(sizeof(d)));

        next();

        return Type::tDbl();
    } else if(testMatchType(TK_CHAR)) {
        *pout<<prefix<<"Matched character literal "<<cur()->str()<<endl;
        t_char c = cur()->str()[0];

        g.addOp(new PushI(sizeof(c), &c), "pushi "+cur()->str()+", size: "+to_string(sizeof(c)));

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
        Symbol *s = sym.get(cur()->str());
        if(s->getType()==SYM_VAR) {
            Variable *v = (Variable*)s;
            t_int addr = v->getAddr();
            addr = addr<0?-addr:addr;
            g.addOp(new PushI(INT_SIZE, &addr), "pushI &"+cur()->str()+": "+to_string(addr));
            next();
            
            if(v->getScope()>1) g.addOp(new AddrVar(), "addrvar");
            
            Type tmp = v->getVarType();
            tmp.reference();
            return tmp;
        } else {
            cout<<"Expected variable, got "<<cur()->str()<<" at ";
            throwError();
        }
    } else if(testMatch("*")) {
        //Dereference pointer
        match("*");
        next();
        Type p = E();
        if(!p.isPtr()) {
            cout<<"Expected pointer for dereference, instead got "<<p<<endl;
            throwError();
        }

        g.addOp(new PushPtr(p.size()), "pushptr "+p.toString()+": "+to_string(p.size()));

        p.dereference();
        return p;
    } else if(testMatchType(TK_IDEN)) {
        //First check if its a typecast
        if(testMatch("int")||testMatch("double")||testMatch("char")) {
            return typecast();
        }
        //Is it variable or function

        Symbol *s = sym.get(cur()->str());
        if(s->getType()==SYM_VAR) {
            Variable *v = (Variable*)s;
            g.addOp(new Push(v->getAddr(), v->getSize()), 
                    "push "+cur()->str()+":"+to_string(v->getAddr())+", size:"+to_string(v->getSize()));
            next();


            return v->getVarType();
        } else if(s->getType()==SYM_FUNC) {
            Function* f = (Function*)s;
			function(f);
            return f->getRet();
        } else {
            cout<<"Expected variable or function, got "<<cur()->str()<<" at ";
            throwError();
        }
    } else if(testMatchType(TK_STR_LIT)){
		StringLit *s = sym.addStringLit(cur()->str());
		int o = s->getOffset();
		g.addOp(new PushI(sizeof(o), &o), "pushi "+to_string(o));
		g.addOp(new PushString(), "pushstring "+cur()->str());
		next();
		
		return Type::charArray();
	}else {
        cout<<"Unexpected token "<<cur()->str()<<" at ";
        throwError();
    }
}

int Parser::compile(const char* fname) {
    sym.dump(*pout);
    g.resolveLabels();
    return g.generate(fname, sym);
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
    Symbol *s = sym.get(varName);
    Variable *var = (Variable*)s;
    Type q = var->getVarType();

    if(!Type::equals(v, q)) {
        cout<<"Can't assign "<<v<<" to "<<q<<" at ";
        throwError();
    }

    g.addOp(new Pop(var->getAddr(), var->getSize()), "pop "+varName+":"+to_string(var->getAddr()));
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

    g.addOp(new PopToPtr(q.size()), "poptoptr "+to_string(q.size()));
}

void Parser::matchControlBlock() {
	bool multiline = false;
	if(testMatch("{")) {
		match("{"); next();
		sym.newScope();
		while(!testMatch("}")) statements();
		int scopeSize = sym.popScope();
		match("}"); next();
		
		if(scopeSize>0) g.addOp(new PopLocal(scopeSize), "poplocal "+to_string(scopeSize));
	}else statements();
}

void Parser::controlStatement(Symbol *s) {
	ControlType c = ((ControlStatement*)s)->type;
	if(c==C_IF) {
		int endLabel = g.nextLabel();
		int nextLabel = g.nextLabel();
		
		bool done = false;
		while(!done) {
			g.addOp(new Label(nextLabel), "L"+to_string(nextLabel));
			nextLabel = g.nextLabel();
				
			match("("); next();
			Type t = E();
			match(")"); next();
			
			if(t!=Type::tChar()) {
				cout<<"Expected char (bool) for if statement, got "<<t<<" at ";
				throwError();
			}
			g.addOp(new PushLbl(nextLabel), "pushlbl L"+to_string(nextLabel)+"\n");
			g.addOp(new IfNJmp(), "if not jump");
			
			
			matchControlBlock();
			g.addOp(new PushLbl(endLabel), "pushlbl L"+to_string(endLabel)+"\n");
			g.addOp(new Jump(), "jmp");
			
			if(testMatch("else")) {
				next();
				if(!testMatch("if")) {
					g.addOp(new Label(nextLabel), "L"+to_string(nextLabel));
					matchControlBlock();
					done = true;
				}else next();
			}else done = true;
		}
		g.addOp(new Label(endLabel), "L"+to_string(endLabel));
	}else if(c==C_WHILE) {
		int endLabel = g.nextLabel();
		int startLabel = g.nextLabel();
		
		//Label beginning of loop
		g.addOp(new Label(startLabel), "L"+to_string(startLabel));
		match("("); next();
		
		Type v = E();
		if(v!=Type::tChar()) {
			cout<<"Expected bool (char) for while statement expression, got "<<v<<" at ";
			throwError();
		}
		
		match(")"); next();
		g.addOp(new PushLbl(endLabel), "pushlbl L"+to_string(endLabel));
		g.addOp(new IfNJmp(), "if not jump");
		
		matchControlBlock();
		g.addOp(new PushLbl(startLabel), "pushlbl L"+to_string(startLabel));
		g.addOp(new Jump(), "jump");
		
		g.addOp(new Label(endLabel), "L"+to_string(endLabel));
	}
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

void Parser::builtInFunction(Symbol *s) {
    BuiltInFunc *f = (BuiltInFunc*)s;
    if(f->func==F_PRINTNUM) {
        next();
        match("(");
        next();
        Type v = E();
        match(")");
        next();

        g.addOp(new PrintNum(v.toSerializedType()), "printnum "+v.toString());
    }else if(f->func==F_PRINT) {
		next();
		match("("); next();
		Type v = E();
		if(v!=Type::charArray()) {
			cout<<"Expected char array got "<<cur()->str()<<" at ";
			throwError();
		}
		match(")"); next();
		
		g.addOp(new Print(), "print");
	}
}

void Parser::statements() {
    *pout<<prefix<<"Current statement: "<<cur()->str()<<endl;
    if(sym.keyExists(cur()->str())) {
        Symbol *s = sym.get(cur()->str());
        SymbolType type = s->getType();
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
			next();
            controlStatement(s);
        } else if(type==SYM_FUNC) {
            function((Function*)s);
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
        if(!currentFunc->retVoid()) E(); //return value
        match(";");
        next();
        
        //Frame stack is handled by return
        //Expression stack doesnt need anything
        //Var stack is the issue
        
        //Return needs to pop all the scopes
        //So we get total local scope size
        int scopeSize = sym.popLocals();
        g.addOp(new PushI(LBL_SIZE, &scopeSize), "pushi:localscopesize "+to_string(scopeSize));
        g.addOp(new Return(), "return");
    }else if(Type::isType(t)){
		//Variable declaration
		var_decl();
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


    sym.addBuiltInFunc("printvar", F_PRINTNUM);
    sym.addBuiltInFunc("print", F_PRINT);
    
    sym.addControlStatement("if", C_IF);
    sym.addControlStatement("while", C_WHILE);
}

void Parser::func_def() {
    
    Type ret;
    bool isVoid = false;
    
    if(!testMatch("void")) ret = Type::parse(t);
    else { isVoid = true; next(); }

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

    Function *f = (Function*)sym.get(cur()->str()); next();
    
    //Check that return value matches
    if(isVoid&&!f->retVoid()) {
        cout<<"Function "<<cur()->str()<<" is not defined as void at ";
        throwError();
    }else if(!isVoid&&ret!=f->getRet()) {
        cout<<"Return value "<<ret<<" does not match "<<f->getRet()<<" at ";
        throwError();
    }
    
    sym.newScope();
    match("("); next();
    
    bool done = false;
    if(testMatch(")")) { done = true; next(); }
    
    //We don't check that args match
    int argsSize = 0;
    list<string> varNames;
    while(!done) {
        Type type = Type::parse(t);
        
        sym.addVar(cur()->str(), type);
        argsSize += type.size();
        
        varNames.push_back(cur()->str()); next();
        
        
        if(testMatch(")")) { done = true; next(); }
        else if(testMatch(",")) next();
        else{
            cout<<"Expected ) or , got "<<cur()->str()<<" at ";
            throwError();
        }
    }
    
    match("{");
    next();

    //Add label
    g.addOp(new Label(f->getLabel()), "L"+to_string(f->getLabel())+":");
    
    //First allocate space on var stack for args then pop into them
    g.addOp(new PushLocal(argsSize), "pushlocal "+to_string(argsSize));
    
    list<Type> *args = f->getArgs();
    auto a = args->rbegin();
    auto n = varNames.rbegin();
    while(a!=args->rend()) {
        Variable *v = (Variable*)sym.get(*n);
        
        g.addOp(new Pop(v->getAddr(), v->getSize()), "pop to arg "+*n+": "+to_string(v->getAddr()));
        
        a++;
        n++;
    }
    
    currentFunc = f;
    
    while(!testMatch("}")) statements();
    match("}");
    next();
    match(";");
    next();
    
    sym.popScope();
}

void Parser::parse() {
    sym.addVar("TMPVAR", Type::tChar());
    g.addOp(new PushLocal(Type::tChar().size()), "allocate empty global var");
    
    
    
    //Globals
    if(testMatch("globals")) {
        next();
        
        while(!testMatch("end")) var_decl();
        next();
    }

    labelMain = g.nextLabel();
    g.addOp(new PushLbl(labelMain), "pushlbl L"+to_string(labelMain));
    
    g.addOp(new Jump(), "jmp");

    //Function prototypes
    if(testMatch("prototypes")) {
        next();
        
        while(!testMatch("end")) {
            func_decl();
        }
        next();
    }
    
    //Function definitions
    if(testMatch("functions")) {
        next();
        
        while(!testMatch("end")) func_def();
        next();
    }

    g.addOp(new Label(labelMain), "L"+to_string(labelMain)+":");
    match("begin"); next();
    while(!testMatch("end")) statements();
    match("end"); next();
    match("EOF");
    next();

    g.addOp(new Halt(), "halt");
}

//<var_decl> -> <type_name> <var_decl_elt>;
//<var_decl_elt> -> ** <var_name> <var_decl_elt_tail>
//<var_decl_elt_tail>, <var_decl_elt> | E
void Parser::var_decl() {
    Type type = Type::parseNoPointer(t);
    *pout<<prefix<<"Variable type is "<<type<<endl;

    bool listDone = false;
    do {
        type.parsePointerLevel(t);

        matchType(TK_IDEN, "Identifier");
        //Check that variable isn't in symbol table or the list
        if(sym.keyExists(cur()->str())) {
            cout<<"Duplicate symbol "<<cur()->str()<<" at ";
            throwError();
        }
        string varName = cur()->str();
        Variable *v = sym.addVar(cur()->str(), type);
        g.addOp(new PushLocal(type.size()), "allocate space for "+cur()->str());
        next();
        
        if(testMatch("=")) {
            next();
            //Assign
            Type eType = E();
            if(type!=eType) {
                cout<<"Can't assign "<<eType<<" to "<<type<<" at ";
                throwError();
            }
            
            g.addOp(new Pop(v->getAddr(), v->getSize()), "pop to "+varName);
        }
        
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
}

void Parser::func_decl() {
    Function *f = new Function();
    
    Type ret;
    if(testMatch("void")) {
        f->setVoid();
        next();
    } else {
        f->setRet(Type::parse(t));
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
    
    
    bool done = false;
    if(testMatch(")")) {
        done = true;
        next();
    }
    while(!done) {
        f->addArg(Type::parse(t));
        
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

    f->setLabel(g.nextLabel());

    sym.addFunc(funcName, f);
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
