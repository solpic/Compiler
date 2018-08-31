OBJ = g++ -c -g -std=c++11 -Wfatal-errors

compiler: Error.o Generator.o main.o Operators.o Parser.o SymTab.o Tokenizer.o
		g++ -g -std=c++11 *.o -o compiler
		
SymTab.o: SymTab.h SymTab.cpp
	$(OBJ) SymTab.cpp
	
Error.o: Error.cpp Error.h Tokenizer.h
	$(OBJ) Error.cpp
	
Generator.o: Generator.cpp Generator.h Operators.h SymTab.h Error.h
	$(OBJ) Generator.cpp
	
main.o: main.cpp Tokenizer.h Parser.h SymTab.h Error.h
	$(OBJ) main.cpp
	
Operators.o: Operators.cpp Operators.h SymTab.h Generator.h
	$(OBJ) Operators.cpp
	
Parser.o: Parser.cpp Parser.h Error.h Tokenizer.h SymTab.h Generator.h Operators.h
	$(OBJ) Parser.cpp
	
Tokenizer.o: Tokenizer.h Tokenizer.cpp Error.h
	$(OBJ) Tokenizer.cpp
	
clean: compiler
	rm compiler
