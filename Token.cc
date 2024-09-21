#include "Token.h"

Token::Token(void)
        : _lex {""},
        _type {TOK_EOF}
{}

Token::Token(int type, const std::string &lex)
        : _lex {lex},
        _type {type}
{
        switch (_type) {
        case TOK_EOF:
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_STAR:
        case TOK_SLASH:
        case TOK_INTLIT:
        case TOK_EQUALS:
        case TOK_IDENT:
        case TOK_PRINT:
        case TOK_INT:
        case TOK_SEMI:
                break;
        default:
                usage("bad token type: %d", _type);
        }
}

int Token::Type(void) const
{
        return _type;
}

std::string Token::Lex(void) const
{
        return _lex;
}


std::string Token::Name(void) const
{
        std::vector<std::string> names {
                "TOK_EOF",
                "TOK_PLUS",
                "TOK_MINUS",
                "TOK_STAR",
                "TOK_SLASH",
                "TOK_INTLIT",
                "TOK_EQUALS",
                "TOK_IDENT",
                "TOK_PRINT",
                "TOK_INT",
                "TOK_SEMI",
        };
        return names[_type];
}
