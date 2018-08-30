#include "Parser.h"
#include "Error.h"

#include <sstream>

std::ostream* Parser::pout = &std::cout;

using namespace std;
//Matching string constant definitions

const char *prefix = "Parser: ";

/*
 * CFG:
 * Program -> <declarations>
 *            <func_defs>
 *            <statements>
 *             EOF
 *
 * <procedure> -> procedure <procedure_name>():
 *                <statements>
 *                end
 *
 * <main> -> begin <statements> end
 * <statements> -> <stmt> <stmt_tail>
 * <stmt_tail> -> E | <statements>
 *
 * <stmt> -> <assignment>
 *           <function_call>
 *           <control>
 * <control> -> <if> <while>
 * <if> -> if <expr>: <statements> end
 * <while> -> while <expr>: <statements> end
 * <assignment> -> <var_name> = <expr>;
 * <function_call> -> <func_name>(<params>)
 * <params> -> <expr> <param_tail>
 * <param_tail> -> , <params> | E
 *
 * <expr> -> <num> | <string>
 * <E> -> <T> <E'>
 * <E'> +<T><E'> | -<T><E'> | empty
 * <T> -> <F> <T'>
 * <T'> -> *<F><T'> | /<F><T'> | empty
 * <F> -> <var> | <literal> | -<F> | +<F> | (<E>)
 *
 *
 * <declarations> -> <decl>+
 *
 * <decl> -> <var_decl>
 *
 * <var_decl> -> <type_name> <var_decl_elt>;
 * <var_decl_elt> -> ** <var_name> <var_decl_elt_tail>
 * <var_decl_elt_tail>, <var_decl_elt> | E
 *
 * <func_decl> -> function: <return_vals> <func_name>(<args>);

 * <func_def> define: <return_vals> <func_name>(<args>) {
 *                  <declarations>
 *                  <statements>
 *                  };

 *
 */

string typeString(ParseVar &p) {
    string s = "";
    for(int i = 0; i<p.pointerLevel; i++) s+="*";
    s+=typeToString(p.v);
    return s;
}

VarType ptrType(ParseVar &p) {
    if(p.pointerLevel>0) return VAR_PTR;
    else return p.v;
}

ParseVar returnVar(VarType v, int pointerLevel) {
    ParseVar p;
    p.v = v;
    p.pointerLevel = pointerLevel;
    return p;
}

int size(ParseVar a) {
    if(a.pointerLevel==0) {
        return varSizes[a.v];
    } else
        return PTR_SIZE;
}

bool typeEqual(ParseVar a, ParseVar b) {
    return (a.v==b.v&&a.pointerLevel==b.pointerLevel);
}

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

bool ptrIntOperatorException(Operator o, ParseVar a, ParseVar b) {
    return (o==ADD||o==SUBTRACT)
           &&(a.pointerLevel>0&&(b.pointerLevel==0&&b.v==VAR_INT));
}

ParseVar Parser::E() {
    ParseVar a = T();
    Operator o;
    string opString = cur()->str();
    while(matchOperatorE(o)) {
        next();
        ParseVar b = T();
        //Check matching types
        if(!ptrIntOperatorException(o, a, b)&&!typeEqual(a, b)) {
            cout<<"Expected appropriate operand type for type "<<typeString(a)<<" and operator "<<opString<<" got "<<typeString(b)<<" at ";
            cur()->tellPositionInformation(cout, t);
            error();
        }
        //next();

        if(o==ADD) {
            g.addOp(new Add(ptrType(a)));
            asmOut += "add\n";
        } else if(o==SUBTRACT) {
            g.addOp(new Sub(ptrType(a)));
            asmOut += "sub\n";
        } else if(o==AND||o==OR) {
            //First check type
            if(a.pointerLevel>0||a.v!=VAR_CHAR) {
                cout<<"Expected char got "<<typeString(a)<<" at ";
                cur()->tellPositionInformation(cout, t);
                error();
            } else if(b.pointerLevel>0||b.v!=VAR_CHAR) {
                cout<<"Expected char got "<<typeString(b)<<" at ";
                cur()->tellPositionInformation(cout, t);
                error();
            }
            if(o==AND) {
                g.addOp(new And(ptrType(a)));
                asmOut += "and\n";
            } else if(o==OR) {
                g.addOp(new Or(ptrType(a)));
                asmOut += "or\n";
            }
            a.pointerLevel = 0;
            a.v = VAR_CHAR;
        }
    }
    return a;
    /*
    char op = cur()->str()[0];
    while(op=='+'||op=='-') {
    next();
    T();

    if(op=='+') {
    	*pout<<prefix<<"Operator +"<<endl;
    	g.addOp(Op::add());
    	asmOut += "add\n";
    }else if(op=='-') {
    	*pout<<prefix<<"Operator -"<<endl;
    	g.addOp(Op::sub());
    	asmOut += "sub\n";
    }

    op = cur()->str()[0];
    }
    */
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

ParseVar Parser::T() {
    ParseVar a = F();
    Operator o;
    string opString = cur()->str();
    while(matchOperatorT(o)) {
        bool isBool = false;
        next();
        ParseVar b = F();
        if(!typeEqual(a, b)) {
            cout<<"Expected "<<typeString(a)<<" got "<<typeString(b)<<" at ";
            cur()->tellPositionInformation(cout, t);
            error();
        }

        if(o==MULT) {
            g.addOp(new Mult(ptrType(a)));
            asmOut += "mult\n";
        } else if(o==DIV) {
            g.addOp(new Div(ptrType(a)));
            asmOut+="div\n";
        } else if(o==EQ) {
            g.addOp(new CmpEq(ptrType(a)));
            isBool = true;
            asmOut+="eq\n";
        } else if(o==NEQ) {
            g.addOp(new CmpNeq(ptrType(a)));
            isBool = true;
            asmOut+= "neq\n";
        } else if(o==GRTR) {
            g.addOp(new CmpGrtr(ptrType(a)));
            isBool = true;
            asmOut += "grtr\n";
        } else if(o==GRTR_EQ) {
            g.addOp(new CmpGrtrEq(ptrType(a)));
            isBool = true;
            asmOut += "grtr_eq\n";
        } else if(o==LESS) {
            g.addOp(new CmpLess(ptrType(a)));
            isBool = true;
            asmOut += "less\n";
        } else if(o==LESS_EQ) {
            g.addOp(new CmpLessEq(ptrType(a)));
            isBool = true;
            asmOut += "less_eq\n";
        }
        if(isBool) {
            a.pointerLevel = 0;
            a.v = VAR_CHAR;
        }
    }
    return a;
    /*
    char op = cur()->str()[0];
    // / * and logical operators
    // || && == !=
    while(op=='*'||op=='/'||op=='|'||op=='&'||op=='='||op=='!'||op=='>'||op=='<') {

    char op2 = 0;
    if(op!='*'&&op!='/') {
    	next();
    	op2 = cur()->str()[0];
    	if(op=='|'&&op2!='|') {
    	cout<<"Expected || at ";
    	cur()->tellPositionInformation(cout, t);
    	error();
    	}else if(op=='&'&&op2!='&') {
    	cout<<"Expected && at ";
    	cur()->tellPositionInformation(cout, t);
    	error();
    	}else if(op=='='&&op2!='=') {
    	cout<<"Expected == at ";
    	cur()->tellPositionInformation(cout, t);
    	error();
    	}else if(op=='!'&&op2!='=') {
    	cout<<"Expected != at ";
    	cur()->tellPositionInformation(cout, t);
    	error();
    	}
    }
    if(!((op=='<'&&op2!='=')||(op=='>'&&op2!='=')))
    	next();
    F();

    if(op=='*') {
    	*pout<<prefix<<"Operator *"<<endl;
    	g.addOp(Op::mult());
    	asmOut += "mult\n";
    }else if(op=='/') {
    	*pout<<prefix<<"Operator /"<<endl;
    	g.addOp(Op::div());
    	asmOut += "div\n";
    }else if(op=='&') {
    	*pout<<prefix<<"Operator &&"<<endl;
    	g.addOp(Op::opAnd());
    	asmOut += "and\n";
    }else if(op=='|') {
    	*pout<<prefix<<"Operator ||"<<endl;
    	g.addOp(Op::opOr());
    	asmOut += "or\n";
    }else if(op=='!') {
    	*pout<<prefix<<"Operator !="<<endl;
    	g.addOp(Op::neq());
    	asmOut += "neq\n";
    }else if(op=='=') {
    	*pout<<prefix<<"Operator =="<<endl;
    	g.addOp(Op::eq());
    	asmOut += "eq\n";
    }else if(op=='<') {
    	if(op2=='=') {
    	g.addOp(Op::lesEq());
    	asmOut += "less-than-eq\n";
    	}else{
    	g.addOp(Op::lessThan());
    	asmOut += "less-than\n";
    	}
    }else if(op=='>') {
    	if(op2=='=') {
    	g.addOp(Op::grEq());
    	asmOut += "greater-than-eq\n";
    	}else{
    	g.addOp(Op::greaterThan());
    	asmOut+="greater-than\n";
    	}
    }
    op = cur()->str()[0];
    }
    */
}

ParseVar Parser::typecast() {
    ParseVar to;
    if(testMatch("int")) to.v = VAR_INT;
    else if(testMatch("double")) to.v = VAR_DOUBLE;
    else if(testMatch("char")) to.v = VAR_CHAR;
    else {
        cout<<"Unrecognized type "<<cur()->str()<<" in typecast at ";
        cur()->tellPositionInformation(cout, t);
        error();
    }
    next();

    to.pointerLevel = 0;
    while(testMatch("*")) {
        to.pointerLevel++;
        next();
    }

    match("(");
    next();
    ParseVar from = E();
    match(")");
    next();

    VarType to_actual = to.pointerLevel>0?VAR_PTR:to.v;
    VarType frm_actual = from.pointerLevel>0?VAR_PTR:from.v;

    if(to_actual!=frm_actual) {
        g.addOp(new Typecast(to_actual, frm_actual));
        asmOut += "typecast "+typeString(to)+" to "+typeString(from)+"\n";
    }

    return to;
}

ParseVar Parser::function(Symbol &s) {
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

    if(!s.isVoid) {
        cout<<"Return: "<<s.returnType.v<<", "<<s.returnType.pointerLevel<<endl;
        return s.returnType;
    } else {
        return returnVar(VAR_VOID, 0);
    }
}

ParseVar Parser::F() {
    //NUM | (E) | VARIABLE | &VARIABLE | STRING_LIT | *E | typecast(E) | true | false | !F | -F
    if(testMatch("!")) {
        *pout<<prefix<<"!"<<endl;
        next();
        if(testMatch("!")) {
            cout<<"! followed by ! is no operation at ";
            cur()->tellPositionInformation(cout, t);
            error();
        }
        ParseVar o = F();
        if(o.pointerLevel!=0||o.v!=VAR_CHAR) {
            cout<<"Expected char for ! operator, got "<<typeString(o)<<" at ";
            cur()->tellPositionInformation(cout, t);
            error();
        }

        g.addOp(new Not());
        asmOut+="not\n";
        return returnVar(VAR_CHAR, 0);
    } else if(testMatch("-")) {
        *pout<<prefix<<"-"<<endl;
        next();
        if(testMatch("-")) {
            cout<<"- followed by - is no operation at ";
            cur()->tellPositionInformation(cout, t);
            error();
        }
        ParseVar o = F();
        if(o.pointerLevel!=0||(o.v!=VAR_INT&&o.v!=VAR_DOUBLE)) {
            cout<<"Expected int or double for - operator, got "<<typeString(o)<<" at ";
            cur()->tellPositionInformation(cout, t);
            error();
        }

        g.addOp(new Negative(ptrType(o)));
        asmOut+="negative\n";
        return o;
    } else if(testMatch("true")||testMatch("false")) {
        *pout<<prefix<<"TRUE/FALSE literal"<<endl;
        t_char c = testMatch("true")?1:0;
        g.addOp(new PushI(sizeof(c), &c));
        asmOut += "pushi "+to_string(c)+"\n";
        next();
        return returnVar(VAR_CHAR, 0);
    } else if(testMatchType(TK_INT)) {
        *pout<<prefix<<"Matched number literal "<<cur()->str()<<endl;
        t_int i = stoi(cur()->str());

        g.addOp(new PushI(sizeof(i), &i));
        asmOut += "pushi "+cur()->str()+", size: "+to_string(sizeof(i))+"\n";

        next();

        return returnVar(VAR_INT, 0);
    } else if(testMatchType(TK_DOUBLE)) {
        *pout<<prefix<<"Matched number literal "<<cur()->str()<<endl;
        t_dbl d = stod(cur()->str());

        g.addOp(new PushI(sizeof(d), &d));
        asmOut += "pushi "+cur()->str()+", size: "+to_string(sizeof(d))+"\n";

        next();

        return returnVar(VAR_DOUBLE, 0);
    } else if(testMatchType(TK_CHAR)) {
        *pout<<prefix<<"Matched character literal "<<cur()->str()<<endl;
        t_char c = cur()->str()[0];

        g.addOp(new PushI(sizeof(c), &c));
        asmOut += "pushi "+cur()->str()+", size: "+to_string(sizeof(c))+"\n";

        next();

        return returnVar(VAR_CHAR, 0);
    } else if(testMatch("(")) {
        match("(");
        next();
        ParseVar v = E();
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
            return returnVar(s.varType, 1);
        } else {
            cout<<"Expected variable, got "<<cur()->str()<<" at ";
            cur()->tellPositionInformation(cout, t);
            error();
        }
    } else if(testMatch("*")) {
        //Dereference pointer
        match("*");
        next();
        ParseVar p = E();
        if(p.pointerLevel<=0) {
            cout<<"Expected pointer for dereference, instead got "<<typeToString(p.v)<<endl;
            error();
        }

        g.addOp(new PushPtr(varSizes[p.v]));
        asmOut += "pushptr "+to_string(p.v)+": "+to_string(varSizes[p.v])+"\n";

        return returnVar(p.v, p.pointerLevel-1);
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


            return returnVar(s.varType, s.pointerLevel);
        } else if(s.type==SYM_FUNC) {
			return function(s);
            /*string fname = cur()->str();
            //Push return vals onto stack
            //Push arguments onto stack
            //Push variables onto stack
            //Push function label onto stack
            //Call
            next();

            if(s.retsScopeSize>0) {
                char returns[s.retsScopeSize];
                g.addOp(new PushI(s.retsScopeSize, returns));
                asmOut += "pushempty:rets "+to_string(s.retsScopeSize)+"\n";
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

            if(s.localsScopeSize>0) {
                char buffer[s.localsScopeSize];
                g.addOp(new PushI(s.localsScopeSize, buffer));
                asmOut += "pushempty:locals "+to_string(s.localsScopeSize)+"\n";
            }

            g.addOp(new PushLbl(s.funcLabel));
            asmOut += "push "+fname+"\n";

            g.addOp(new Call());
            asmOut += "call\n";

            if(!s.isVoid) {
                return s.returnType;
            } else {
                return returnVar(VAR_VOID, 0);
            }*/
        } else {
            cout<<"Expected variable or function, got "<<cur()->str()<<" at ";
            cur()->tellPositionInformation(cout, t);
            error();
        }
    } else {
        cout<<"Unexpected token "<<cur()->str()<<" at ";
        cur()->tellPositionInformation(cout, t);
        sym.dump(cout);
        error();
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
    //cout<<"Current: "<<cur()->str()<<" at ";
    // cur()->tellPositionInformation(cout, t);
    //cout<<"Advancing"<<endl;
    return t->advance();
}

void Parser::match(const char *s) {
    if(!cur()->equals(s)) {
        cout<<"Expected token "<<s<<", got "<<cur()->str()<<" at ";
        cur()->tellPositionInformation(cout, t);
        error();
    } else {
        *pout<<prefix<<"Matched "<<s<<endl;
    }
}

void Parser::matchType(TokenType type, const char *expected) {
    if(cur()->getType()!=type) {
        cout<<"Expected type "<<type<<", got "<<cur()<<" at ";
        cur()->tellPositionInformation(cout, t);
        error();
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

    ParseVar v = E();
    Symbol s = sym.get(varName);

    ParseVar q;
    q.v = s.varType;
    q.pointerLevel = s.pointerLevel;

    if(!typeEqual(v, q)) {
        cout<<"Can't assign "<<typeString(v)<<" to "<<typeString(q)<<" at ";
        cur()->tellPositionInformation(cout, t);
        error();
    }

    g.addOp(new Pop(s.getAddr(), s.size()));
    asmOut+="pop "+varName+":"+to_string(s.getAddr())+"\n";
}

void Parser::pointerAssignment() {
    ParseVar v = E();
    match("=");
    next();
    ParseVar q = E();
    v.pointerLevel--;
    if(!typeEqual(v, q)) {
        cout<<"Can't dereference "<<typeString(q)<<" into "<<typeString(v)<<" at ";
        cur()->tellPositionInformation(cout, t);
        error();
    }

    g.addOp(new PopToPtr(size(q)));
    asmOut+="poptoptr "+to_string(size(q))+"\n";
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
        ParseVar v = E();
        match(")");
        next();

        g.addOp(new PrintNum(ptrType(v)));
        asmOut+="printnum "+typeString(v)+"\n";
    }
    /*
    else if(s.builtInFunc==F_PRINT) {
    next();
    match("(");
    next();
    E();
    match(")");
    next();
    g.addOp(Op::print());
    asmOut+="print\n";
    }else if(s.builtInFunc==F_READINT) {
    next();
    match("("); next();
    match(")"); next();

    g.addOp(Op::readInt());
    asmOut+="readint\n";
    }
    */
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

        /*else if(type==Procedure) {
        	next();
        	match("("); next();
        	match(")"); next();
        	match(";"); next();

        	int retLbl = g.curLabel++;
        	g.addOp(Op::pushLbl(retLbl));
        	asmOut+="pushlbl L"+to_string(retLbl)+"\n";
        	g.addOp(Op::pushLbl(s.procLabel));
        	asmOut+="pushlbl "+cur()->str()+":L"+to_string(s.procLabel)+"\n";
        	g.addOp(Op::opJmp());
        	asmOut+="jmp\n";

        	g.addOp(Op::label(retLbl));
        	asmOut+="L"+to_string(retLbl)+"\n";
        }*/
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
            cur()->tellPositionInformation(cout, t);
            error();
        }
        //Evaluate return arguments and push them into old stack
        //for(int i = 0; i<currentFunc->numRet; i++) {
        if(!currentFunc->isVoid) {
            int i = 0;
            E();
            Symbol ret = sym.get(to_string(i));

            g.addOp(new Pop(ret.addr, ret.size()));
            asmOut += "pop return value "+to_string(i)+"\n";
            //}
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
    }/*else if(testMatch("}")){
		if(!currentFunc){
		cout<<"Can't return without enclosing function at ";
		cur()->tellPositionInformation(cout, t);
		error();
	}

	int scope = currentFunc->localsScopeSize + currentFunc->argsScopeSize;
	g.addOp(new PushI(LBL_SIZE, &scope));
	asmOut += "pushi "+to_string(scope)+", size: "+to_string(LBL_SIZE)+"\n";

	g.addOp(new Return());
	asmOut += "return\n";
	}*/else {
        cout<<"Expected statement, got "<<cur()->str()<<" at ";
        cur()->tellPositionInformation(cout, t);
        sym.dump(cout);
        error();
    }
}

void Parser::main() {
    /*
    match("begin");
    next();

    g.addOp(new Label());
    asmOut += "L"+to_string(labelMain)+":\n";

    statements();
    match("end");
    next();

    g.addOp(new Halt());
    asmOut += "halt\n";
     */
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


void Parser::procedures() {
    /*
    while(testMatch("procedure")) {
    match("procedure"); next();

    string procName = cur()->str();
    next();
    Symbol s(Procedure);

    if(sym.keyExists(procName)) {
    	cout<<"Duplicate symbol "<<procName<<" at ";
    	cur()->tellPositionInformation(cout, t);
    	error();
    }else{
    	int label = g.curLabel++;
    	s.procLabel = label;
    	sym.addSymbol(procName, s);

    	match("("); next();
    	match(")"); next();
    	match(":"); next();

    	g.addOp(Op::label(label));
    	asmOut+="L"+to_string(label)+":\n";

    	statements();

    	match("end"); next();

    	g.addOp(Op::opJmp());
    	asmOut+="jmp\n";

    	*pout<<prefix<<"End of procedure"<<endl;
    }
    }
    */
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
        cur()->tellPositionInformation(cout, t);
        error();
        return;
    }

    //Check that it exists
    if(!sym.keyExists(cur()->str())) {
        cout<<"Trying to define undeclared function "<<cur()->str()<<" at ";
        cur()->tellPositionInformation(cout, t);
        error();
        return;
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
            cur()->tellPositionInformation(cout, t);
            error();
            return;
        }
        if(sym.keyExists(cur()->str())) {
            cout<<"Duplicate symbol "<<cur()->str()<<" at ";
            cur()->tellPositionInformation(cout, t);
            error();
            return;
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
    /*
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
    ret.varType = parseToVar(s->returnType);
    ret.pointerLevel = s->returnType.pointerLevel;

    sym.addSymbol("0", ret);
    //}

    int retScopeSize = sym.scopeSize;

    for(i = 0; i<s->numArg; i++) {
        Symbol arg(SYM_VAR);
        arg.varType = s->args[i].v;
        arg.pointerLevel = pointerLevels[i];

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

/*
void Parser::str_decl() {
	matchType(Identifier, "identifier");

	string name = cur()->str();
	if(sym.keyExists(name)) {
	cout<<"Duplicate symbol "<<name<<" at ";
	cur()->tellPositionInformation(cout, t);
	error();
	}else{
	next();
	match(":");
	next();
	matchType(Str, "string literal");
	Symbol s(StringLiteral);
	s.lit = new char[cur()->str().length()];
	strcpy(s.lit, cur()->str().c_str());

	sym.addSymbol(name, s);
	next();

	match(";"); next();
	}

}
 * */

//<var_decl> -> <type_name> <var_decl_elt>;
//<var_decl_elt> -> ** <var_name> <var_decl_elt_tail>
//<var_decl_elt_tail>, <var_decl_elt> | E
void Parser::var_decl(VarType varType) {
    *pout<<prefix<<"Variable type is "<<cur()->str()<<endl;
    next();

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
            cur()->tellPositionInformation(cout, t);
            error();
            return;
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
            cur()->tellPositionInformation(cout, t);
            error();
        }
    } while(!listDone);

    //Now we actually add the variables
    list<string>::iterator i = varNames.begin();
    list<int>::iterator j = pointerLevels.begin();
    while(i!=varNames.end()) {
        Symbol s(SYM_VAR);
        s.varType = varType;
        s.pointerLevel = *j;

        sym.addSymbol(*i, s);

        i++;
        j++;
    }
}

ParseVar Parser::parseFunctionType() {
    VarType v;
    if(testMatch("int")) v = VAR_INT;
    else if(testMatch("double")) v = VAR_DOUBLE;
    else if(testMatch("char")) v = VAR_CHAR;
    else {
        cout<<"Expected variable return type, got "<<cur()->str()<<" at ";
        cur()->tellPositionInformation(cout, t);
        error();
    }
    next();
    int pointerLevel = 0;
    while(testMatch("*")) {
        pointerLevel++;
        next();
    }

    return returnVar(v, pointerLevel);
}

void Parser::func_decl() {
	next();
    match(":");
    next();
    //list<Variable> retVals;
    //list<Variable> args;

    //Return values
    /*while(!testMatch(")")) {
    Variable v;
    if(testMatch("int")) {
    	v.type = VAR_INT;
    	retVals.push_back(v);
    }else if(testMatch("double")) {
    	v.type = VAR_DOUBLE;
    	retVals.push_back(v);
    }else if(testMatch("char")) {
    	v.type = VAR_CHAR;
    	retVals.push_back(v);
    }
    next();
    //This allows for trailing comma
    if(testMatch(", ")) next();
    }*/
    ParseVar ret;
    bool isVoid = false;
    if(testMatch("void")) {
        isVoid = true;
        next();
    } else {
        ret = parseFunctionType();
    }

    //func name
    if(!testMatchType(TK_IDEN)) {
        cout<<"Expected identifier, got "<<cur()->str()<<" at ";
        cur()->tellPositionInformation(cout, t);
        error();
        return;
    }

    //Check key is unique
    if(sym.keyExists(cur()->str())) {
        cout<<"Key "<<cur()->str()<<" already exists ";
        cur()->tellPositionInformation(cout, t);
        error();
        return;
    }

    string funcName = cur()->str();
    next();

    match("(");
    next();
    //Match arguments
    list<ParseVar> args;
    bool done = false;
    if(testMatch(")")) {
        done = true;
        next();
    }
    while(!done) {
        args.push_back(parseFunctionType());
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
            cur()->tellPositionInformation(cout, t);
            error();
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
    s.args = new ParseVar[args.size()];
    int j = 0;
    for(auto i: args) s.args[j++] = i;

    s.funcLabel = g.curLabel++;
    sym.addSymbol(funcName, s);
}

void Parser::declarations(bool funcs) {
    //Different types of declarations
    bool declarationsDone = false;
    while(!declarationsDone) {
        if(testMatch("int")) {
            //Int declaration
            var_decl(VAR_INT);
        } else if(testMatch("double")) {
            var_decl(VAR_DOUBLE);
        } else if(testMatch("char")) {
            var_decl(VAR_CHAR);
        } else if(funcs&&testMatch("function")) {
            //Function declaration
            func_decl();
        } else {
            declarationsDone = true;
        }
    }
}
