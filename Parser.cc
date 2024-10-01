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

                if (left == nullptr) {
                        left = tree;
                } else {
                        left = new Ast{AST_GLUE, TYPE_NONE,
                                left, nullptr, tree, 0};
                }

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
        int i;

        switch (_lex.Curr().Type()) {
        case TOK_INTLIT:
                i = atoi(_lex.Curr().Lex().c_str());
                if (i >= 0 && i < 256)
                        n = new Ast{AST_INTLIT, TYPE_CHAR, i};
                else
                        n = new Ast{AST_INTLIT, TYPE_INT, i};
                break;
        case TOK_IDENT:
                s = _cg.GetGlo(_lex.Curr().Lex());
                n = new Ast{AST_IDENT, s->Prim(), s->Name()};
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

                auto lefttype = left->Dtype();
                auto righttype = right->Dtype();
                if (!type_compat(&lefttype, &righttype, 0)) {
                        usage("incompatible types %s and %s",
                                        type_name(lefttype).c_str(),
                                        type_name(righttype).c_str());
                }

                if (lefttype)
                        left = new Ast{lefttype, right->Dtype(), left, 0};
                if (righttype)
                        right = new Ast{righttype, left->Dtype(), right, 0};

                left = new Ast{tok2ast(tt), left->Dtype(), left, right, 0};
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

        int lefttype = TYPE_INT;
        auto righttype = expr->Dtype();
        if (!type_compat(&lefttype, &righttype, 0)) {
                usage("type %s not compatible with %s",
                                type_name(lefttype).c_str(),
                                type_name(righttype).c_str());
        }

        if (righttype)
                expr = new Ast{righttype, TYPE_INT, expr, 0};

        expr = new Ast{AST_PRINT, TYPE_NONE, expr, 0};
        return expr;
}

void Parser::parseVarDecl(void)
{
        int type;

        switch (_lex.Curr().Type()) {
        case TOK_CHAR:
                type = TYPE_CHAR;
                break;
        case TOK_INT:
                type = TYPE_INT;
                break;
        case TOK_VOID:
                type = TYPE_VOID;
                break;
        }
        _lex.Eat(_lex.Curr().Type());

        auto id = _lex.Curr().Lex();
        _lex.Eat(TOK_IDENT);
        _cg.SetGlo(type, STYPE_VAR, id);
        _cg.GenGlo(id);
        _lex.Eat(TOK_SEMI);
}

Ast *Parser::parseAssign(void)
{
        auto id = _lex.Curr().Lex();

        _lex.Eat(TOK_IDENT);

        auto s = _cg.GetGlo(id);
        auto right = new Ast{AST_LVIDENT, s->Prim(), s->Name()};
        _lex.Eat(TOK_EQUALS);

        auto left = parseExpr(0);

        auto lefttype = left->Dtype();
        auto righttype = right->Dtype();
        if (!type_compat(&lefttype, &righttype, 1)) {
                usage("incompatible types %s and %s",
                                type_name(lefttype).c_str(),
                                type_name(righttype).c_str());
        }

        if (lefttype)
                left = new Ast{lefttype, right->Dtype(), left, 0};

        auto tree = new Ast{AST_ASSIGN, TYPE_INT, left, right, 0};
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
        return new Ast{AST_IF, TYPE_NONE, cond, truetree, falsetree, 0};
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
        return new Ast{AST_WHILE, TYPE_NONE, cond, nullptr, body, 0};
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
        auto tree = new Ast{AST_GLUE, TYPE_NONE, body, nullptr, post, 0};
        tree = new Ast{AST_WHILE, TYPE_NONE, cond, nullptr, tree, 0};
        return new Ast{AST_GLUE, TYPE_NONE, pre, nullptr, tree, 0};
}

Ast *Parser::parseSingle(void)
{
        switch (_lex.Curr().Type()) {
        case TOK_PRINT:
                return parsePrint();
        case TOK_CHAR:
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

Ast *Parser::ParseFuncDecl(void)
{
        _lex.Eat(TOK_VOID);
        auto id = _lex.Curr().Lex();
        _lex.Eat(TOK_IDENT);
        _cg.SetGlo(TYPE_VOID, STYPE_FUNC, id);
        _lex.Eat(TOK_LPAREN);
        _lex.Eat(TOK_RPAREN);
        auto n = ParseCompound();
        return new Ast{AST_FUNC, TYPE_NONE, n, id};
}
