#include "Hooks.h"

HookCode matchHookCode(string name) {
	if(name=="malloc") return HOOK_MALLOC;
	if(name=="delete") return HOOK_DELETE;
}

void Hook::h_malloc(Emulator &e) {
	t_int size;
	e.pop(sizeof(size), &size);
	
	t_ptr t = (t_ptr)malloc(size);
	t = -t;
	
	e.push(sizeof(t), &t);
}

void Hook::h_del(Emulator &e) {
	t_ptr ptr;
	e.pop(sizeof(ptr), &ptr);
	void *v = (void*)(-ptr);
	free(v);
}

void Hook::run(Emulator &e) {
	HookCode hook;
	e.pop(sizeof(hook), &hook);
	
	switch(hook) {
		case HOOK_MALLOC:
		h_malloc(e);
		break;
		case HOOK_DELETE:
		h_del(e);
		break;
	}
	
	e.ip += opSize();
}
