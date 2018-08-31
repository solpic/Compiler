#include <stdio.h>
#include <iostream>

#include "Tokenizer.h"
#include "Parser.h"
#include "SymTab.h"
#include "Error.h"

using namespace std;

int main(int argc, char **argv)
{
    if(argc==1) {
        cout<<"Please specify a source file as the second argument"<<endl;
        cout<<"For verbose mode, add the \"verbose\" flag as well"<<endl;
        return 0;
    }
    //Enter name of source file here
    char* srcFile = argv[1];
    //Enter name of output file here
    char* outFile = "output.out";
    int quiet = 1;
    cout<<"Quiet? ";
    cin>>quiet;
    //if(argc==3) { quiet = strcmp("verbose", argv[2])!=0; }
    //quiet = 0;
    //(argc<3||strcmp("verbose", argv[2])!=0);
    if(quiet) {
        Parser::setOutputStream(garbageStream());
        Emulator::setOutput(garbageStream());
        SymTab::setOutput(garbageStream());
    }
    Tokenizer t(srcFile);
    Parser p(&t);

    p.parse();

    if(!quiet) {
        cout<<endl<<"ASM:"<<endl;
        p.dumpAsm();
        cout<<endl;
    }
    p.compile(outFile);

    Emulator e(outFile);

    return 0;
}
