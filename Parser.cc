#include "Parser.h"

std::string func_id;

static int tok2prim(Lexer &lex, int tok)
{
        int type;

        if (tok == TOK_CHAR) {
                type = TYPE_CHAR;
        } else if (tok == TOK_INT) {
                type = TYPE_INT;
        } else if (tok == TOK_LONG) {
                type = TYPE_LONG;
        } else if (tok == TOK_VOID) {
                type = TYPE_VOID;
        } else {
                usage("%s not a primitive", Token{tok, ""}.Name().c_str());
                exit(EXIT_FAILURE);
        }

        for (;;) {
                lex.Next();
                if (lex.Curr().Type() != TOK_STAR)
                        break;
                type = ptr_to(type);
        }

        return type;
}

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

                if (tree != nullptr) {
                        if (tree->Type() == AST_PRINT ||
                            tree->Type() == AST_ASSIGN ||
                            tree->Type() == AST_RETURN ||
                            tree->Type() == AST_CALL) {
                                _lex.Eat(TOK_SEMI);
                        }
                }

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
        std::string id;
        int i;

        switch (_lex.Curr().Type()) {
        case TOK_INTLIT:
                i = atoi(_lex.Curr().Lex().c_str());
                if (i >= 0 && i < 256)
                        n = new Ast{AST_INTLIT, TYPE_CHAR, i};
                else
                        n = new Ast{AST_INTLIT, TYPE_INT, i};
                _lex.Next();
                break;
        case TOK_IDENT:
                id = _lex.Curr().Lex();
                _lex.Eat(TOK_IDENT);

                if (_lex.Curr().Type() == TOK_LPAREN)
                        return parseCall(id);

                s = _cg.GetGlo(id);
                n = new Ast{AST_IDENT, s->Prim(), s->Name()};
                break;
        default:
                usage("invalid primary: %s", _lex.Curr().Name().c_str());
        }

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

        if (type < 0 || type > (int)(sizeof(prec) / sizeof(*prec)))
                usage("invalid operator: %s", Token{type, ""}.Name().c_str());

        auto p = prec[type];
        if (p == 0)
                usage("invalid operator: %s", Token{type, ""}.Name().c_str());

        return p;
}

Ast *Parser::parseExpr(int ptp)
{
        auto left = parsePrefix();
        auto tt = _lex.Curr().Type();
        if (tt == TOK_SEMI || tt == TOK_RPAREN)
                return left;

        while (op_prec(tt) > ptp) {
                _lex.Next();
                auto right = parseExpr(op_prec(tt));

                auto op = tok2ast(tt);
                auto ltmp = modify_type(_cg, left, right->Dtype(), op);
                auto rtmp = modify_type(_cg, right, left->Dtype(), op);
                if (ltmp == nullptr && rtmp == nullptr) {
                        puts("parseExpr()");
                        usage("incompatible types");
                }

                if (ltmp != nullptr)
                        left = ltmp;
                if (rtmp != nullptr)
                        right = rtmp;

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

        printf("%s\n", type_name(expr->Dtype()).c_str());
        expr = modify_type(_cg, expr, TYPE_INT, 0);
        if (expr == nullptr) {
                puts("parsePrint()");
                usage("incompatible types");
        }

        expr = new Ast{AST_PRINT, TYPE_NONE, expr, 0};
        return expr;
}

void Parser::parseVarDecl(int type, const std::string& id)
{
        auto ident = std::string{id};

        for (;;) {
                _cg.SetGlo(type, STYPE_VAR, 0, ident);
                _cg.GenGlo(ident);
                if (_lex.Curr().Type() == TOK_SEMI) {
                        _lex.Eat(TOK_SEMI);
                        break;
                }
                if (_lex.Curr().Type() == TOK_COMMA) {
                        _lex.Eat(TOK_COMMA);
                        ident = _lex.Curr().Lex();
                        _lex.Eat(TOK_IDENT);
                        continue;
                }
                usage("missing , or ; after identifier");
        }
}

Ast *Parser::parseAssign(void)
{
        auto id = _lex.Curr().Lex();

        _lex.Eat(TOK_IDENT);

        if (_lex.Curr().Type() == TOK_LPAREN)
                return parseCall(id);

        auto s = _cg.GetGlo(id);
        auto right = new Ast{AST_LVIDENT, s->Prim(), s->Name()};
        _lex.Eat(TOK_EQUALS);

        auto left = parseExpr(0);

        left = modify_type(_cg, left, right->Dtype(), 0);
        if (left == nullptr) {
                puts("parseAssign()");
                usage("incompatible types");
        }

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
        std::string id;
        int type;

        switch (_lex.Curr().Type()) {
        case TOK_PRINT:
                return parsePrint();
        case TOK_CHAR:
        case TOK_INT:
        case TOK_LONG:
                type = tok2prim(_lex, _lex.Curr().Type());
                id = _lex.Curr().Lex();
                _lex.Eat(TOK_IDENT);
                parseVarDecl(type, id);
                return nullptr;
        case TOK_IDENT:
                return parseAssign();
        case TOK_IF:
                return parseIf();
        case TOK_WHILE:
                return parseWhile();
        case TOK_FOR:
                return parseFor();
        case TOK_RETURN:
                return parseRet();
        default:
                usage("bad token: %s", _lex.Curr().Name().c_str());
                exit(1);
        }
}

Ast *Parser::ParseFuncDecl(int type, const std::string& id)
{
        func_id = id;

        auto end = _cg.GetLabel();
        _cg.SetGlo(type, STYPE_FUNC, end, id);

        _lex.Eat(TOK_LPAREN);
        _lex.Eat(TOK_RPAREN);
        auto n = ParseCompound();

        if (type != TYPE_VOID) {
                if (n == nullptr)
                        usage("empty non-void function");
                auto fin = n;
                if (n->Type() == AST_GLUE)
                        fin = n->Right();
                if (fin == nullptr || fin->Type() != AST_RETURN)
                        usage("no return for non-void function");
        }

        return new Ast{AST_FUNC, type, n, id};
}

Ast *Parser::parseRet(void)
{
        auto s = _cg.GetGlo(func_id);
        if (s->Prim() == TOK_VOID)
                usage("returning item from void function");

        _lex.Eat(TOK_RETURN);
        _lex.Eat(TOK_LPAREN);
        auto tree = parseExpr(0);

        tree = modify_type(_cg, tree, s->Prim(), 0);
        if (tree == nullptr) {
                puts("parseRet()");
                usage("incompatible types");
        }

        tree = new Ast{AST_RETURN, TYPE_NONE, tree, 0};
        _lex.Eat(TOK_RPAREN);
        return tree;
}

Ast *Parser::parseCall(const std::string& id)
{
        auto s = _cg.GetGlo(id);
        _lex.Eat(TOK_LPAREN);
        auto tree = parseExpr(0);
        tree = new Ast{AST_CALL, s->Prim(), tree, id};
        _lex.Eat(TOK_RPAREN);
        return tree;
}

Ast *Parser::parsePrefix(void)
{
        Ast *n;

        switch (_lex.Curr().Type()) {
        case TOK_AMPER:
                _lex.Next();
                n = parsePrefix();

                if (n->Type() != AST_IDENT)
                        usage("applying & to non-identifier");

                n->SetType(AST_ADDR);
                n->SetDtype(ptr_to(n->Dtype()));
                break;
        case TOK_STAR:
                _lex.Next();
                n = parsePrefix();

                if (n->Type() != AST_IDENT && n->Type() != AST_DEREF)
                        usage("* followed by ident or *");

                n = new Ast{AST_DEREF, val_at(n->Dtype()), n, 0};
                break;
        default:
                n = parsePrimary();
                break;
        }

        return n;
}

void Parser::ParseDecls(void)
{
        for (;;) {
                auto type = tok2prim(_lex, _lex.Curr().Type());
                auto id = _lex.Curr().Lex();
                _lex.Eat(TOK_IDENT);
                if (_lex.Curr().Type() == TOK_LPAREN) {
                        auto n = ParseFuncDecl(type, id);
                        _cg.GenAst(n, NIL_REG, 0);
                        astfree(n);
                } else {
                        parseVarDecl(type, id);
                }
                if (_lex.Curr().Type() == TOK_EOF)
                        break;
        }
}
