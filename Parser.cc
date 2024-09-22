#include "Parser.h"

Parser::Parser(Lexer &lex, CodeGen &cg)
        : _cg {cg},
        _lex {lex}
{
        _lex.Next();
}

void Parser::ParseStmt(void)
{
        switch (_lex.Curr().Type()) {
        case TOK_PRINT:
                parse_print();
                break;
        case TOK_INT:
                parse_var_decl();
                break;
        case TOK_IDENT:
                parse_assign();
                break;
        case TOK_EOF:
                return;
        default:
                usage("invalid token type: %s", _lex.Curr().Name().c_str());
        }
}

Ast *Parser::parse_primary(void)
{
        Sym *s;
        Ast *n;

        switch (_lex.Curr().Type()) {
        case TOK_INTLIT:
                n = new Ast{AST_INTLIT, atoi(_lex.Curr().Lex().c_str())};
                break;
        case TOK_IDENT:
                s = _cg.GetGlo(_lex.Curr().Lex());
                n = new Ast{AST_IDENT, s->Name()};
                break;
        default:
                usage("invalid primary: %s", _lex.Curr().Name().c_str());
        }

        _lex.Next();
        return n;
}

static int tok2ast(int type)
{
        if (type > TOK_EOF && type < TOK_INTLIT)
                return type;
        usage("invalid operator: %s", Token{type, ""}.Name().c_str());
        exit(EXIT_FAILURE);
}

static int op_prec(int type)
{
        static int prec[] = {
                0,      // TOK_EOF
                10,     // TOK_PLUS
                10,     // TOK_MINUS
                20,     // TOK_STAR
                20,     // TOK_SLASH
                30,     // TOK_EQ
                30,     // TOK_NE
                40,     // TOK_LT
                40,     // TOK_GT
                40,     // TOK_LE
                40,     // TOK_GE
        };

        auto p = prec[type];
        if (p == 0)
                usage("invalid operator: %s", Token{type, ""}.Name().c_str());

        return p;
}

Ast *Parser::parse_expr(int ptp)
{
        auto left = parse_primary();
        auto tt = _lex.Curr().Type();
        if (tt == TOK_SEMI)
                return left;

        while (op_prec(tt) > ptp) {
                _lex.Next();
                auto right = parse_expr(op_prec(tt));
                left = new Ast{tok2ast(tt), left, right, 0};
                tt = _lex.Curr().Type();
                if (tt == TOK_SEMI)
                        return left;
        }

        return left;
}

void Parser::parse_print(void)
{
        _lex.Eat(TOK_PRINT);
        auto n = parse_expr(0);
        auto reg = _cg.GenAst(n, (size_t)-1);
        _cg.GenPrintInt(reg);
        _cg.Free();
        _lex.Eat(TOK_SEMI);
        astfree(n);
}

void Parser::parse_var_decl(void)
{
        _lex.Eat(TOK_INT);
        auto id = _lex.Curr().Lex();
        _lex.Eat(TOK_IDENT);
        _cg.SetGlo(id);
        _cg.GenGlo(id);
        _lex.Eat(TOK_SEMI);
}

void Parser::parse_assign(void)
{
        auto id = _lex.Curr().Lex();
        _lex.Eat(TOK_IDENT);
        auto s = _cg.GetGlo(id);
        auto right = new Ast{AST_LVIDENT, s->Name()};
        _lex.Eat(TOK_EQUALS);
        auto left = parse_expr(0);
        auto root = new Ast{AST_ASSIGN, left, right, 0};
        _cg.GenAst(root, (size_t)-1);
        _cg.Free();
        _lex.Eat(TOK_SEMI);
        astfree(root);
}
