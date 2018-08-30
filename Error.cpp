#include "Error.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

void error() {
    exit(-1);
}

void error(char *msg) {
    cout<<msg<<endl;
    exit(-1);
}

void error(const std::string &s) {
    cout<<s<<endl;
    exit(-1);
}

NullBuffer nullBuffer;
std::ostream null_stream(&nullBuffer);

std::ostream* garbageStream() {
    return &null_stream;
    
}