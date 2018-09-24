OBJ2 = g++ -c -g -std=c++11
OBJ3 = g++ -c -g -std=c++11 -Wfatal-errors 
OBJ = g++ -c -g -std=c++11 -fmax-errors=4

compiler: Error.o Generator.o main.o Operators.o Parser.o SymTab.o Tokenizer.o Type.o Hooks.o
		g++ -g -std=c++11 *.o -o compiler
		
Hooks.o: Hooks.h Hooks.cpp Generator.h
	$(OBJ) Hooks.cpp
		
Type.o: Type.h Type.cpp Tokenizer.h Error.h SymTab.h
	$(OBJ) Type.cpp

SymTab.o: SymTab.h SymTab.cpp Type.h
	$(OBJ) SymTab.cpp
	
Error.o: Error.cpp Error.h Tokenizer.h
	$(OBJ) Error.cpp
	
Generator.o: Generator.cpp Generator.h Operators.h SymTab.h Error.h Hooks.h
	$(OBJ) Generator.cpp
	
main.o: main.cpp Tokenizer.h Parser.h SymTab.h Error.h
	$(OBJ) main.cpp
	
Operators.o: Operators.cpp Operators.h SymTab.h Generator.h
	$(OBJ) Operators.cpp
	
Parser.o: Parser.cpp Parser.h Error.h Tokenizer.h SymTab.h Generator.h Operators.h Type.h Hooks.h
	$(OBJ) Parser.cpp
	
Tokenizer.o: Tokenizer.h Tokenizer.cpp Error.h
	$(OBJ) Tokenizer.cpp
	
clean: compiler
	rm compiler
	rm *.o
