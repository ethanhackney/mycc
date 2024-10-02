#ifndef LEXER_H
#define LEXER_H

#include "Error.h"
#include "Token.h"
#include <cctype>
#include <cstdio>
#include <string>

// lexical analyzer
class Lexer {
private:
        std::string     _path;  // path name of file being read
        Token           _curr;  // current token
        Token           _rej;   // rejected token
        FILE            *_fp;   // FILE to read from
        int             _pc;    // putback character

        // get next char from input
        int nextchar(void);

        // read an integer literal
        std::string readint(int c);

        // read an identifier
        std::string readid(int c);
public:
        // @path:       path name of file to read
        Lexer(const std::string &path);

        // get current token
        Token Curr(void) const;

        // get next token
        Token Next(void);

        // skip if token type is type or error out
        void Eat(int type);

        // add token back into input
        void Reject(Token tok);

        ~Lexer();
};

#endif
