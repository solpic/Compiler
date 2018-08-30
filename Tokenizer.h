#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <iostream>
#include <fstream>

class Tokenizer;

enum TokenType{TK_IDEN=0, TK_STR_LIT=1, TK_INT=2, TK_OP=3, TK_EOF=4, TK_UNKNOWN=5, TK_CHAR=6, TK_DOUBLE=7};
std::ostream & operator<<(std::ostream &out, const TokenType &t);
    
class Token{
private:
    TokenType type;
    //Actual string of token
    std::string readString;
    
    //Used for debugging
    int lineNumber;
    int lineIndex;
    int absoluteIndex;
public:
    bool equals(const char *s) const{
        return readString.compare(s)==0;
    }
    std::string str() const { return readString; }
    Token() {}
    TokenType getType() { return type; }
    
    Token(TokenType t, std::string s, int lN, int lI, int aI) {
        type = t;
        
        //Positions for debugging
        readString = s;
        lineNumber = lN;
        lineIndex = lI;
        absoluteIndex = aI;
    }
    void tellPositionInformation(std::ostream &o, Tokenizer *t);
    friend std::ostream & operator<<(std::ostream &out, const Token &t);
};

class Tokenizer{
private:
    std::ifstream ifile;
    Token *curToken;
    int lineNumber;
    int lineIndex;
    int filePos;
    
    char getChar();
    char peekChar();
    void consumeWhitespaceAndComments();
public:
    Tokenizer(char *fname);
    ~Tokenizer();
    
    Token* getNextToken();
    Token* getCurToken();
    
    Token* cur() { return getCurToken(); }
    Token* advance() { return getNextToken(); }
    void tellPositionInformation(std::ostream &o, int lN, int lI, int aI, int len);
    void tellCurPosInformation(std::ostream &o, int offset, int len);
};

#endif