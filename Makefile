compiler: Error.cpp Error.h Generator.cpp Generator.h main.cpp Operators.cpp Operators.h Parser.cpp \
			Parser.h SymTab.cpp SymTab.h Tokenizer.cpp Tokenizer.h
		g++ -g -std=c++11 *.cpp -o compiler

clean:
	rm compiler
