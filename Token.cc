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
        case TOK_EQ:
        case TOK_NE:
        case TOK_LT:
        case TOK_GT:
        case TOK_LE:
        case TOK_GE:
        case TOK_INTLIT:
        case TOK_SEMI:
        case TOK_EQUALS:
        case TOK_IDENT:
        case TOK_LBRACE:
        case TOK_RBRACE:
        case TOK_LPAREN:
        case TOK_RPAREN:
        case TOK_PRINT:
        case TOK_INT:
        case TOK_IF:
        case TOK_ELSE:
        case TOK_WHILE:
        case TOK_FOR:
        case TOK_VOID:
        case TOK_CHAR:
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
                "TOK_EQ",
                "TOK_NE",
                "TOK_LT",
                "TOK_GT",
                "TOK_LE",
                "TOK_GE",
                "TOK_INTLIT",
                "TOK_SEMI",
                "TOK_EQUALS",
                "TOK_IDENT",
                "TOK_LBRACE",
                "TOK_RBRACE",
                "TOK_LPAREN",
                "TOK_RPAREN",
                "TOK_PRINT",
                "TOK_INT",
                "TOK_IF",
                "TOK_ELSE",
                "TOK_WHILE",
                "TOK_FOR",
                "TOK_VOID",
                "TOK_CHAR",
        };
        return names[_type];
}
