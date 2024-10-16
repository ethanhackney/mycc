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
        case TOK_ASSIGN:
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
        case TOK_VOID:
        case TOK_CHAR:
        case TOK_INT:
        case TOK_LONG:
        case TOK_INTLIT:
        case TOK_SEMI:
        case TOK_IDENT:
        case TOK_LBRACE:
        case TOK_RBRACE:
        case TOK_LPAREN:
        case TOK_RPAREN:
        case TOK_IF:
        case TOK_ELSE:
        case TOK_WHILE:
        case TOK_FOR:
        case TOK_RETURN:
        case TOK_AMPER:
        case TOK_LOGAND:
        case TOK_COMMA:
        case TOK_LBRACK:
        case TOK_RBRACK:
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
                "TOK_ASSIGN",
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
                "TOK_VOID",
                "TOK_CHAR",
                "TOK_INT",
                "TOK_LONG",
                "TOK_INTLIT",
                "TOK_SEMI",
                "TOK_IDENT",
                "TOK_LBRACE",
                "TOK_RBRACE",
                "TOK_LPAREN",
                "TOK_RPAREN",
                "TOK_LBRACK",
                "TOK_RBRACK",
                "TOK_AMPER",
                "TOK_LOGAND",
                "TOK_IF",
                "TOK_ELSE",
                "TOK_WHILE",
                "TOK_FOR",
                "TOK_RETURN",
                "TOK_COMMA",
        };
        return names[_type];
}
