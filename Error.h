#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include "Tokenizer.h"

void error();
void error(char *msg);
void error(const std::string &s);
//void error_expect(const char *exp, const Token *t);

class NullBuffer : public std::streambuf
{
public:
  int overflow(int c) { return c; }
};

std::ostream* garbageStream();

#endif
