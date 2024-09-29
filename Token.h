#ifndef TOKEN_H
#define TOKEN_H

#include "Error.h"
#include <string>
#include <vector>

// token types
enum {
        TOK_EOF,        // end of file
        TOK_PLUS,       // '+'
        TOK_MINUS,      // '-'
        TOK_STAR,       // '*'
        TOK_SLASH,      // '/'
        TOK_EQ,         // '=='
        TOK_NE,         // '!='
        TOK_LT,         // '<'
        TOK_GT,         // '>'
        TOK_LE,         // '<='
        TOK_GE,         // '>='
        TOK_INTLIT,     // integer literal
        TOK_SEMI,       // ';'
        TOK_EQUALS,     // '='
        TOK_IDENT,      // identifier
        TOK_LBRACE,     // '{'
        TOK_RBRACE,     // '}'
        TOK_LPAREN,     // '('
        TOK_RPAREN,     // ')'
        TOK_PRINT,      // 'print'
        TOK_INT,        // 'int'
        TOK_IF,         // 'if'
        TOK_ELSE,       // 'else'
        TOK_WHILE,      // 'while'
        TOK_FOR,        // 'for'
        TOK_VOID,       // 'void'
};

// token
class Token {
private:
        std::string     _lex;   // lexeme
        int             _type;  // token type
public:
        // default constructor
        Token(void);

        // @type:       token type
        // @lex:        lexeme
        Token(int type, const std::string &lex);

        // get token type
        int Type(void) const;

        // get lexeme
        std::string Lex(void) const;

        // get token type name
        std::string Name(void) const;
};

#endif
