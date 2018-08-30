#include "Tokenizer.h"
#include "Error.h"

using namespace std;

const char *tokenTypeStrings[] = {"Identifier", "String", "Integer", "Operand", "EOF", "Unknown", "Char", "Double"};

std::ostream & operator<<(std::ostream &out, const TokenType &t) {
    out<<tokenTypeStrings[t];
    return out;
}

std::ostream & operator<<(std::ostream &out, const Token &t) {
    out<<"Type: "<<tokenTypeStrings[t.type]<<", Value: "<<t.readString<<std::endl;
    return out;
}

Tokenizer::Tokenizer(char* fname) {
    ifile.open(fname,ios::binary);
    
    lineNumber = 1;
    lineIndex = 0;
    curToken = 0;
    filePos = 0;
}

char Tokenizer::getChar() {
    char c = ifile.get();
    filePos++;
    if(c=='\n') {
        lineNumber++;
        lineIndex = 0;
    }else lineIndex++;
    return c;
}

char Tokenizer::peekChar() {
    return (char)ifile.peek();
}

bool isWhitespace(char c) {
    return c==' '||c=='\t'||c=='\n'||c=='\r';
}

void Tokenizer::consumeWhitespaceAndComments() {
    bool done = false;
    while(!done) {
        //Whitespace
        char c = peekChar();
        while(isWhitespace(c)) {
            getChar();
            c = peekChar();
        }
        
        //Comment
        int pos = ifile.tellg();
        bool comment = false;
        if(c=='/') {
            getChar();
            c = peekChar();
            if(c=='/') {
                //Single line comment
                comment = true;
                while(c!='\n') c = getChar();
            }else if(c=='*') {
                getChar();
                //Multiline comment
                bool endOfComment = false;
                comment = true;
                while(!endOfComment) {
                    c = getChar();
                    if(c=='*'&&peekChar()=='/') {
                        endOfComment = true;
                        getChar();
                    }
                }
            }else{
                ifile.seekg(pos);
            }
        }
        
        if(!comment) done = true;
    }
}

bool isAlphabet(char c) {
    return (c>='a'&&c<='z')||(c>='A'&&c<='Z');
}

bool isDigit(char c) {
    return (c>='0'&&c<='9');
}

bool isNumeric(char c) {
    return isDigit(c)||c=='.';
}

bool isIdentifierChar(char c) {
    return isAlphabet(c)||c=='_'||isDigit(c);
}

char *operandStrings[] = {"=", "+", "-", "*", "%", "/", "(", ")", "{", "}", "[", "]", ";",
",", ":", "<", ">", "||", "&&", "!", "!=", "&", "==", "<=", ">="};
int numOpStrings = 25;
char operandChars[] = {'=', '+', '-', '*', '%', '/', '(', ')', '{', 
                '}', '[', ']', ';', ',', ':', '<', '>', '|', '&', '!'};
int numOpChars = 20;

bool isOperand(char c) {
    for(int i = 0; i<numOpChars; i++)
        if(c==operandChars[i]) return true;
    return false;
}

bool makesOperand(string &s) {
    for(char *op: operandStrings) 
        if(s.compare(op)==0) return true;
    return false;
}

char escape(char c) {
    if(c=='n') return '\n';
    if(c=='\t') return '\t';
    if(c=='\'') return '\'';
    if(c=='\"') return '\"';
    return 0;
}

void Tokenizer::tellCurPosInformation(ostream &o, int offset, int len) {
    tellPositionInformation(o, lineNumber, lineIndex, ifile.tellg()-offset, len);
}

Token* Tokenizer::getNextToken() {
    if(curToken) {
        delete curToken;
        curToken = 0;
    }
    consumeWhitespaceAndComments();
    
    string tok;
    
    TokenType tokType = TK_UNKNOWN;
    //Save position for debugging
    int tokenLineNumber = lineNumber;
    int tokenLineIndex = lineIndex;
    int tokenAbsoluteIndex = ifile.tellg();
    
    tok.push_back(getChar());
    if(tok[0]==EOF||ifile.eof()) {
        tok = "EOF";
        tokType = TK_EOF;
    }else if(isOperand(tok[0])) {
        //Read in an operator
        //Max operator length is two
        char c = peekChar();
        string tmp = tok + c;
        if(makesOperand(tmp)) {
            //Two char operand
            getChar();
            tok = tmp;
        }else if(!makesOperand(tok)) {
            //Unknown op
            cout<<"Expected operator, got "<<tok<<" at ";
            tellCurPosInformation(cout, 0, 1);
            error();
            return 0;
        }
        
        tokType = TK_OP;
    }else if(isAlphabet(tok[0])) {
        //Identifiers & keywords
        char c = peekChar();
        while(isIdentifierChar(c)) {
            tok.push_back(getChar());
            c = peekChar();
        }
        
        tokType = TK_IDEN;
    }else if(isNumeric(tok[0])) {
        //Numbers
        char c = peekChar();
        while(isNumeric(c)) {
            tok.push_back(getChar());
            c = peekChar();
        }
        
        if(tok.find('.', 0)==tok.npos)
            tokType = TK_INT;
        else tokType = TK_DOUBLE;
    }else if(tok[0]=='\''){
        //Character literal
        tok = "";
        char c = getChar();
        if(c=='\\') {
            c = getChar();
            char e = escape(c);
            if(!e) {
                cout<<"Unrecognized escape sequence \\"<<c<<" at ";
                tellCurPosInformation(cout, 1, 2);
                error();
                return 0;
            }else
                c = e;
        }
        tok += c;
        c = getChar();
        if(c!='\'') {
            cout<<"Expected \', got "<<c<<" at ";
            tellCurPosInformation(cout, 1, 2);
            error();
            return 0;
        }
        
        tokType = TK_CHAR;
    }else if(tok[0]=='"') {
        //String literals
        tok = "";
        char c = getChar();
        while(c!='"') {
            tok.push_back(c);
            //Handle escapes
            if(c=='\\') {
                c = getChar();
                char e = escape(c);
                if(!e) {
                    cout<<"Unrecognized escape sequence \\"<<c<<" at ";
                    tellCurPosInformation(cout, 1, 2);
                    error();
                    return 0;
                }else{
                    tok.push_back(e);
                }
            }
            
            c = getChar();
        }
        
        tokType = TK_STR_LIT;
    }else{
        cout<<"Unrecognized character "<<(int)tok[0]<<" at ";
        tellCurPosInformation(cout, 0, 1);
        error();
        return 0;
    }
    
    Token *t = new Token(tokType, tok, tokenLineNumber, tokenLineIndex, tokenAbsoluteIndex);
    curToken = t;
    return t;
}

//Debugging output
void Tokenizer::tellPositionInformation(ostream &o, int lN, int lI, int aI, int len) {
    int curPos = ifile.tellg();
    
    //Seek beginning of line
    ifile.seekg(aI-lI);
    string tmp;
    if(!getline(ifile, tmp)) error("Couldn't read line for debugging");
    
    o<<"line number: "<<lN<<", index: "<<lI<<endl;
    //o<<tmp.substr(0, lI-1)<<"**"<<tmp.substr(lI, len)<<"**"<<tmp.substr(lI+len, string::npos)<<endl;
    o<<tmp<<endl;
    
    ifile.seekg(curPos);
}


void Token::tellPositionInformation(std::ostream &o, Tokenizer *t) {
    t->tellPositionInformation(o, lineNumber, lineIndex, absoluteIndex, readString.length());
}

Token* Tokenizer::getCurToken() {
    if(curToken==0) return getNextToken();
    else return curToken;
}

Tokenizer::~Tokenizer() {
    ifile.close();
}