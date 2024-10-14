#ifndef TOKEN_H
#define TOKEN_H

#include "Error.h"
#include <string>
#include <vector>

// token types
enum {
        TOK_EOF,        // end of file
        TOK_ASSIGN,     // "="
        TOK_PLUS,       // "+"
        TOK_MINUS,      // "-"
        TOK_STAR,       // "*"
        TOK_SLASH,      // "/"
        TOK_EQ,         // "=="
        TOK_NE,         // "!="
        TOK_LT,         // "<"
        TOK_GT,         // ">"
        TOK_LE,         // "<="
        TOK_GE,         // ">="
        TOK_VOID,       // "void"
        TOK_CHAR,       // "char"
        TOK_INT,        // "int"
        TOK_LONG,       // "long"
        TOK_INTLIT,     // integer literal
        TOK_SEMI,       // ";"
        TOK_IDENT,      // identifier
        TOK_LBRACE,     // "{"
        TOK_RBRACE,     // "}"
        TOK_LPAREN,     // "("
        TOK_RPAREN,     // ")"
        TOK_LBRACK,     // "["
        TOK_RBRACK,     // "]"
        TOK_AMPER,      // "&"
        TOK_LOGAND,     // "&&"
        TOK_IF,         // "if"
        TOK_ELSE,       // "else"
        TOK_WHILE,      // "while"
        TOK_FOR,        // "for"
        TOK_RETURN,     // "return"
        TOK_COMMA,      // ","
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
