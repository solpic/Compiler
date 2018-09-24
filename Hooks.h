#ifndef HOOKS_H
#define HOOKS_H
#include "Generator.h"
#include <string>

class Hook: public Op{
public:
	Hook() {code = OP_HOOK;}
	void run(Emulator &e);
	
	void h_malloc(Emulator &e);
	void h_del(Emulator &e);
};

HookCode matchHookCode(std::string s);

#endif
