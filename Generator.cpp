#include "Generator.h"
#include "Operators.h"
#include <fstream>

using namespace std;

ostream* Emulator::eout = &cout;
string Emulator::prefix = "Emulator: ";
Op* Op::opFromCode(int code) {
	switch(code) {
	case op_pushlbl:
	return new PushLbl();
	break;
	
	case op_label:
	return new Label();
	break;
	
	case op_push:
	return new Push();
	break;
	
	case op_pop:
	return new Pop();
	break;
	
	case op_printint:
	return new PrintNum();
	break;
	
	case op_pushi:
	return new PushI();
	break;
	
	case op_jmp:
	return new Jump();
	break;
	
	case op_halt:
	return new Halt();
	break;
	
	case OP_CALL:
	return new Call();
	break;
	
	case OP_RETURN:
	return new Return();
	break;
	
	case op_add:
	return new Add();
	break;
	
	case op_sub:
	return new Sub();
	break;
	
	case op_mult:
	return new Mult();
	break;
	
	case op_div:
	return new Div();
	break;
	
	case op_eq:
	return new CmpEq();
	break;
	
	case op_neq:
	return new CmpNeq();
	break;
	
	case op_and:
	return new And();
	break;
	
	case op_or:
	return new Or();
	break;
	
	case op_grtr:
	return new CmpGrtr();
	break;
	
	case op_grteq:
	return new CmpGrtrEq();
	break;
	
	case op_lessthan:
	return new CmpLess();
	break;
	
	case op_leseq:
	return new CmpLessEq();
	break;
	
	case OP_NEG:
	return new Negative();
	break;
	
	case OP_NOT:
	return new Not();
	break;
	
	case OP_POPPTR:
	return new PopToPtr();
	break;
	
	case OP_REFPTR:
	return new PushPtr();
	break;
	
	case OP_CAST:
	return new Typecast();
	break;
	
	default:
	cout<<"Unrecognized op code "<<code<<endl;
	error();
	return 0;
	}
}

void Return::run(Emulator &e) {
	//Pop scope size
	//Pop return location
	//Pop old stack pointer
	//Update SP
	//Jump
	
	LBL scopeSize;
	LBL sp_tmp;
	LBL ret_ip;
	e.pop(LBL_SIZE, &scopeSize);
	e.pop(LBL_SIZE, &ret_ip);
	e.pop(LBL_SIZE, &sp_tmp);
	
	
	e.stk.resize(e.stk.size()-scopeSize);
	e.sp = sp_tmp;
	e.ip = ret_ip;
	
}

void Call::run(Emulator &e) {
	//Pop function label
	//Push old stack pointer
	//Push return location
	//Update SP
	//Execute jmp
	
	LBL sp_tmp = e.sp;
	LBL fnc_ip;
	LBL ret_ip = e.ip + opSize();
	e.pop(LBL_SIZE, &fnc_ip);
	
	e.push(LBL_SIZE, &sp_tmp);
	e.push(LBL_SIZE, &ret_ip);
	
	/*
	e.pop(LBL_SIZE, &scopeSize);
	e.pop(LBL_SIZE, &ret_ip);
	e.pop(LBL_SIZE, &sp_tmp);
	*/
	
	e.sp = e.stk.size();
	e.ip = fnc_ip;
}

void Jump::run(Emulator &e) {
	int n_ip;
	e.pop(LBL_SIZE, &n_ip);
	
	e.ip = n_ip;
}

void PushLbl::run(Emulator &e){
	e.push(LBL_SIZE, &lbl);
	
	e.ip += opSize();
}

void Pop::run(Emulator &e) {
	if(addr>=0) {
	e.pop(size, e.code+addr);
	}else{
	e.pop(size, &e.stk[e.sp+addr]);
	}
	
	e.ip += opSize();
}
template <class T, class F>
int cast(F frm, char *data) {
	T t = (T)frm;
	memcpy(data, &t, sizeof(T));
	return sizeof(T);
}

void Typecast::run(Emulator &e) {
	char data[20];
	int size = 0;
	if(from==VAR_INT&&to!=VAR_INT) {
	t_int f;
	e.pop(INT_SIZE, &f);
	
	if(to==VAR_CHAR) size = cast<t_char, t_int>(f, data);
	else if(to==VAR_DOUBLE) size = cast<t_dbl, t_int>(f, data);
	else if(to==VAR_PTR) size = cast<t_ptr, t_int>(f, data);
	}else if(from==VAR_CHAR&&to!=VAR_CHAR) {
	t_char f;
	e.pop(CHAR_SIZE, &f);
	
	if(to==VAR_INT) size = cast<t_int, t_char>(f, data);
	else if(to==VAR_DOUBLE) size = cast<t_dbl, t_char>(f, data);
	else if(to==VAR_PTR) size = cast<t_ptr, t_char>(f, data);
	}else if(from==VAR_DOUBLE&&to!=VAR_DOUBLE) {
	t_dbl f;
	e.pop(DBL_SIZE, &f);
	
	if(to==VAR_INT) size = cast<t_int, t_dbl>(f, data);
	else if(to==VAR_CHAR) size = cast<t_char, t_dbl>(f, data);
	else if(to==VAR_PTR) size = cast<t_ptr, t_dbl>(f, data);
	}else if(from==VAR_PTR&&to!=VAR_PTR) {
	t_ptr f;
	e.pop(PTR_SIZE, &f);
	
	if(to==VAR_INT) size = cast<t_int, t_ptr>(f, data);
	else if(to==VAR_CHAR) size = cast<t_char, t_ptr>(f, data);
	else if(to==VAR_DOUBLE) size = cast<t_dbl, t_ptr>(f, data);
	}
	
	if(size>0) {
	e.push(size, data);
	}else{
	cout<<"Unrecognized cast from "<<from<<" to "<<to<<endl;
	error();
	}
	
	e.ip += opSize();
}

void PopToPtr::run(Emulator &e) {
	t_ptr c;
	char *tmp = new char[size];
	e.pop(size, tmp);
	e.pop(PTR_SIZE, &c);
	
	if(c>=0) {
	memcpy(e.code+c, tmp, size);
	}else{
	memcpy(&e.stk[e.sp+c], tmp, size);
	}
	
	e.ip += opSize();
}

void PushPtr::run(Emulator &e) {
	t_ptr c;
	e.pop(PTR_SIZE, &c);
	
	if(c>=0) {
	e.push(size, e.code+c);
	}else{
	e.push(size, &e.stk[e.sp+c]);
	}
	
	e.ip += opSize();
}

void Push::run(Emulator &e) {
	*e.eout<<"Pushing from addr: "<<addr<<" of size "<<size<<", SP is "<<e.sp<<endl<<endl;
	if(addr>=0) {
	e.push(size, e.code+addr);
	}else{
	e.push(size, &e.stk[e.sp+addr]);
	int i = e.sp+addr;
	*e.eout<<"Starting at position "<<i<<": ";
	for(int j = 0; j<size; j++) {
		*e.eout<<(int)e.stk[i+j]<<" ";
	}
	*e.eout<<endl<<endl;
	}
	
	e.ip += opSize();
}

void PrintNum::run(Emulator &e) {
	*e.eout<<endl<<"PROGRAM OUTPUT: ";
	if(type==VAR_INT) {
	t_int i;
	e.pop(sizeof(t_int), &i);
	
	cout<<i<<endl;
	}else if(type==VAR_DOUBLE) {
	t_dbl d;
	e.pop(sizeof(t_dbl), &d);
	
	cout<<d<<endl;
	}else if(type==VAR_CHAR){
	t_char c;
	e.pop(sizeof(t_char), &c);
	
	cout<<c<<endl;
	}else{
	cout<<"PRINTNUM: Unrecognized type"<<endl;
	error();
	}
	
	*e.eout<<endl;
	e.ip += opSize();
}

void PushI::run(Emulator &e) {
	e.push(size, value);
	
	e.ip += opSize();
}

void Emulator::push(int size, void *val) {
	//We have to copy val before we resize the stack, because val might be in the stack
	char *valCpy = new char[size];
	memcpy(valCpy, val, size);
	
	int i = stk.size();
	stk.resize(stk.size()+size);
	memcpy(&stk[i], valCpy, size);
	
	delete[] valCpy;
}

void Emulator::pop(int size, void *val) {
	int i = stk.size();
	memcpy(val, &stk[i-size], size);
	stk.resize(i-size);
}

void Generator::resolveLabels(int dataSize) {
	int *labels = new int[curLabel];
	for(int i = 0; i<curLabel; i++) labels[i] = -1;
	
	int pos = 0;
	pos += sizeof(DataHeader) + dataSize;
	
	//Calculate positions
	for(auto i = ops.begin(); i!=ops.end(); i++) {
	Op *o = *i;
	if(o->code!=op_label) pos += o->opSize();
	else {
		Label *lbl = (Label*)o;
		labels[lbl->lbl] = pos;
	}
	}
	
	//Substitute in for jump statements
	for(auto i = ops.begin(); i!=ops.end(); i++) {
	int j = (*i)->code;
	
	if(j==op_pushlbl) {
		PushLbl *l = (PushLbl*)*i;
		int k = l->lbl;
		l->lbl = labels[k];
		*Emulator::eout<<"Label "<<k<<" resolved to "<<l->lbl<<endl;
	}
	}
}

void Generator::generate(const char *fname, int dataSize, SymTab &sym) {
	ofstream of;
	of.open(fname, ofstream::out | ofstream::trunc | ofstream::binary);
	
	DataHeader d;
	d.size = dataSize;
	char *c = new char[dataSize+sizeof(d)];
	memcpy(c, &d, sizeof(d));
	sym.fillStringLiterals(c);
	of.write(c, dataSize+sizeof(d));
	
	//Now code segment
	char buffer[100];
	for(auto i = ops.begin(); i!=ops.end(); i++) {
	//Ignore labels
	Op *o = *i;
	if(o->code!=op_label) {
		of.write((char*)&o->code, sizeof(o->code));
		o->putData(buffer);
		of.write(buffer, o->dataSize());
	}
	delete *i;
	}
	
	of.close();
}

void Emulator::printStack() {
	*eout<<prefix<<"Stack Unformatted: ";
	for(char c: stk) *eout<<(int)c<<" ";
	*eout<<endl;
	
	int pos = 0;
	for(auto it = stkSizes.begin(); it!=stkSizes.end(); it++) {
	pos += *it;
	}
	if(pos!=stk.size()) {
	*eout<<"Stack formatting alignment error"<<endl;
	return;
	}
	*eout<<"Stack Formatted: \n";
	pos = 0;
	for(auto it = stkSizes.begin(); it!=stkSizes.end(); it++) {
	*eout<<*it<<": ";
	if(*it==DBL_SIZE) {
		t_dbl b;
		memcpy(&b, &stk[pos], *it);
		*eout<<"D "<<b<<" | ";
	}else if(*it==INT_SIZE) {
		t_int a;
		memcpy(&a, &stk[pos], *it);
		*eout<<"I "<<a<<" | ";
	}
	for(int j = 0; j<*it; j++) {
		*eout<<(int)stk[j+pos]<<" ";
	}
	*eout<<endl;
	pos += *it;
	}
}

void Emulator::run() {
	stk.resize(0);
	ip = 0;
	
	DataHeader d;
	memcpy(&d, code, sizeof(DataHeader));
	
	data = sizeof(DataHeader);
	ip = data+d.size;
	
	//Set IP to first op
	bool halt = false;
	do{
	int opcode;
	memcpy(&opcode, code+ip, sizeof(opcode));
	
	//Debugging
	int initialStkSize = stk.size();
	*eout<<prefix<<"OP: "<<opcode<<endl;
	//*eout<<prefix<<"IP: "<<ip<<endl;
	printStack();
	*eout<<endl;
	
	Op *o = Op::opFromCode(opcode);
	o->parseData(code+ip+sizeof(opcode));
	o->run(*this);
	
	//More debugging
	int dStkSize = stk.size() - initialStkSize;
	if(dStkSize>0)
		stkSizes.push_back(dStkSize);
	else
		stkSizes.pop_back();
	
	halt = o->code==op_halt;
	delete o;
	}while(!halt);
}

Emulator::Emulator(const char *fname) {
	ifstream ifile(fname, ifstream::in | ifstream::binary);
	
	ifile.seekg(0, ifile.end);
	int len = ifile.tellg();
	ifile.seekg(0, ifile.beg);
	
	code = new char[len];
	ifile.read(code, len);
	ifile.close();
	
	run();
	
	delete[] code;
}

Emulator::~Emulator() {
	
}