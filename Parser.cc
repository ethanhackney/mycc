#include "Parser.h"

Parser::Parser(Lexer &lex, CodeGen &cg)
        : _cg {cg},
        _lex {lex}
{
        _lex.Next();
}

Ast *Parser::ParseCompound(void)
{
        Ast *left {nullptr};
        Ast *tree;

        _lex.Eat(TOK_LBRACE);

        for (;;) {
                tree = parseSingle();

                if (tree != nullptr && (tree->Type() == AST_PRINT ||
                                        tree->Type() == AST_ASSIGN))
                        _lex.Eat(TOK_SEMI);

                if (tree == nullptr)
                        continue;

                if (left == nullptr)
                        left = tree;
                else
                        left = new Ast{AST_GLUE, left, nullptr, tree, 0};

                if (_lex.Curr().Type() == TOK_RBRACE) {
                        _lex.Eat(TOK_RBRACE);
                        return left;
                }
        }

        return tree;
}

Ast *Parser::parsePrimary(void)
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

Ast *Parser::parseExpr(int ptp)
{
        auto left = parsePrimary();
        auto tt = _lex.Curr().Type();
        if (tt == TOK_SEMI || tt == TOK_RPAREN)
                return left;

        while (op_prec(tt) > ptp) {
                _lex.Next();
                auto right = parseExpr(op_prec(tt));
                left = new Ast{tok2ast(tt), left, right, 0};
                tt = _lex.Curr().Type();
                if (tt == TOK_SEMI || tt == TOK_RPAREN)
                        return left;
        }

        return left;
}

Ast *Parser::parsePrint(void)
{
        _lex.Eat(TOK_PRINT);
        auto expr = parseExpr(0);
        auto pr = new Ast{AST_PRINT, expr, 0};
        return pr;
}

void Parser::parseVarDecl(void)
{
        _lex.Eat(TOK_INT);
        auto id = _lex.Curr().Lex();
        _lex.Eat(TOK_IDENT);
        _cg.SetGlo(id);
        _cg.GenGlo(id);
        _lex.Eat(TOK_SEMI);
}

Ast *Parser::parseAssign(void)
{
        auto id = _lex.Curr().Lex();
        _lex.Eat(TOK_IDENT);
        auto s = _cg.GetGlo(id);
        auto right = new Ast{AST_LVIDENT, s->Name()};
        _lex.Eat(TOK_EQUALS);
        auto left = parseExpr(0);
        auto tree = new Ast{AST_ASSIGN, left, right, 0};
        return tree;
}

Ast *Parser::parseIf(void)
{
        _lex.Eat(TOK_IF);
        _lex.Eat(TOK_LPAREN);
        auto cond = parseExpr(0);
        if (cond->Type() < AST_EQ || cond->Type() > AST_GE)
                usage("parseIf: invalid comparison operator");
        _lex.Eat(TOK_RPAREN);
        auto truetree = ParseCompound();
        Ast *falsetree {nullptr};
        if (_lex.Curr().Type() == TOK_ELSE) {
                _lex.Eat(TOK_ELSE);
                falsetree = ParseCompound();
        }
        return new Ast{AST_IF, cond, truetree, falsetree, 0};
}

Ast *Parser::parseWhile(void)
{
        _lex.Eat(TOK_WHILE);
        _lex.Eat(TOK_LPAREN);
        auto cond = parseExpr(0);
        if (cond->Type() < AST_EQ || cond->Type() > AST_GE)
                usage("invalid comparison op: %s", cond->Name().c_str());
        _lex.Eat(TOK_RPAREN);
        auto body = ParseCompound();
        return new Ast{AST_WHILE, cond, nullptr, body, 0};
}

Ast *Parser::parseFor(void)
{
        _lex.Eat(TOK_FOR);
        _lex.Eat(TOK_LPAREN);
        auto pre = parseSingle();
        _lex.Eat(TOK_SEMI);
        auto cond = parseExpr(0);
        if (cond->Type() < AST_EQ || cond->Type() > AST_GE)
                usage("bad comparison operator: %s", cond->Name().c_str());
        _lex.Eat(TOK_SEMI);
        auto post = parseSingle();
        _lex.Eat(TOK_RPAREN);
        auto body = ParseCompound();
        auto tree = new Ast{AST_GLUE, body, nullptr, post, 0};
        tree = new Ast{AST_WHILE, cond, nullptr, tree, 0};
        return new Ast{AST_GLUE, pre, nullptr, tree, 0};
}

Ast *Parser::parseSingle(void)
{
        switch (_lex.Curr().Type()) {
        case TOK_PRINT:
                return parsePrint();
        case TOK_INT:
                parseVarDecl();
                return nullptr;
        case TOK_IDENT:
                return parseAssign();
        case TOK_IF:
                return parseIf();
        case TOK_WHILE:
                return parseWhile();
        case TOK_FOR:
                return parseFor();
        default:
                usage("bad token: %s", _lex.Curr().Name().c_str());
                exit(1);
        }
}
