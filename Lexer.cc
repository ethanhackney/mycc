#include "Lexer.h"

Lexer::Lexer(const std::string &path)
        : _path {path},
        _curr {},
        _fp {fopen(path.c_str(), "r")},
        _pc {'\0'}
{
        if (_fp == nullptr)
                error("could not open: %s", path.c_str());
}

Token Lexer::Curr(void) const
{
        return _curr;
}

int Lexer::nextchar(void)
{
        auto c = _pc;

        if (c != '\0')
                _pc = '\0';
        else
                c = fgetc(_fp);

        if (ferror(_fp))
                error("could not read: %s", _path.c_str());

        return c;
}

std::string Lexer::readint(int c)
{
        std::string n;

        while (isdigit(c)) {
                n += c;
                c = nextchar();
        }
        _pc = c;

        return n;
}

std::string Lexer::readid(int c)
{
        std::string w;

        while (isalpha(c) || isdigit(c) || c == '_') {
                w += c;
                c = nextchar();
        }
        _pc = c;

        return w;
}

Token Lexer::Next(void)
{
        for (;;) {
                auto c = nextchar();

                if (isspace(c))
                        continue;

                switch (c) {
                case EOF:
                        return _curr = Token{TOK_EOF, ""};
                case '+':
                        return _curr = Token{TOK_PLUS, "+"};
                case '-':
                        return _curr = Token{TOK_MINUS, "-"};
                case '*':
                        return _curr = Token{TOK_STAR, "*"};
                case '/':
                        return _curr = Token{TOK_SLASH, "/"};
                case '=':
                        if ((c = nextchar()) == '=') {
                                return _curr = Token{TOK_EQ, "=="};
                        }
                        _pc = c;
                        return _curr = Token{TOK_EQUALS, "="};
                case '!':
                        if ((c = nextchar()) == '=')
                                return _curr = Token{TOK_NE, "!="};
                        usage("bad character: %c", c);
                case '<':
                        if ((c = nextchar()) == '=')
                                return _curr = Token{TOK_LE, "<="};
                        _pc = c;
                        return _curr = Token{TOK_LT, "<"};
                case '>':
                        if ((c = nextchar()) == '=')
                                return _curr = Token{TOK_GE, ">="};
                        _pc = c;
                        return _curr = Token{TOK_GT, ">"};
                case ';':
                        return _curr = Token{TOK_SEMI, ";"};
                case '{':
                        return _curr = Token{TOK_LBRACE, "{"};
                case '}':
                        return _curr = Token{TOK_RBRACE, "}"};
                case '(':
                        return _curr = Token{TOK_LPAREN, "("};
                case ')':
                        return _curr = Token{TOK_RPAREN, ")"};
                }

                if (isdigit(c)) {
                        auto n = readint(c);
                        return _curr = Token{TOK_INTLIT, n};
                } else if (isalpha(c) || c == '_') {
                        auto w = readid(c);

                        if (w == "print")
                                return _curr = Token{TOK_PRINT, w};
                        if (w == "int")
                                return _curr = Token{TOK_INT, w};
                        if (w == "if")
                                return _curr = Token{TOK_IF, w};
                        if (w == "else")
                                return _curr = Token{TOK_ELSE, w};
                        if (w == "while")
                                return _curr = Token{TOK_WHILE, w};
                        if (w == "for")
                                return _curr = Token{TOK_FOR, w};
                        if (w == "void")
                                return _curr = Token{TOK_VOID, w};
                        if (w == "char")
                                return _curr = Token{TOK_CHAR, w};

                        return _curr = Token{TOK_IDENT, w};
                } else {
                        usage("invalid character: %c", c);
                }
        }
}

Lexer::~Lexer()
{
        if (fclose(_fp) < 0)
                error("could not close %s", _path.c_str());
}

void Lexer::Eat(int type)
{
        if (_curr.Type() == type) {
                Next();
                return;
        }
        error("expected %s, got %s", Token{type, ""}.Name().c_str(),
                        _curr.Name().c_str());
}
