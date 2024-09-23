#include "Ast.h"

// check if ast type is valid
//
// @type:       type to check
static void typeok(int type)
{
        switch (type) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_INTLIT:
        case AST_IDENT:
        case AST_LVIDENT:
        case AST_ASSIGN:
        case AST_EQ:
        case AST_NE:
        case AST_LT:
        case AST_GT:
        case AST_LE:
        case AST_GE:
        case AST_PRINT:
        case AST_GLUE:
        case AST_IF:
                break;
        default:
                usage("invalid ast type: %d", type);
        }
}

Ast::Ast(void)
        : _left {nullptr},
        _right {nullptr},
        _mid {nullptr},
        _type {AST_NONE}
{}


Ast::Ast(int type, Ast *left, Ast *right, int intlit)
        : _left {left},
        _right {right},
        _mid {nullptr},
        _type {type},
        _intlit {intlit}
{
        typeok(_type);
}

Ast::Ast(int type, Ast *left, Ast *right, const std::string &id)
        : _id {id},
        _left {left},
        _right {right},
        _mid {nullptr},
        _type {type}
{
        typeok(_type);
}

Ast::Ast(int type, int intlit)
        : _left {nullptr},
        _right {nullptr},
        _mid {nullptr},
        _type {type},
        _intlit {intlit}
{
        typeok(_type);
}

Ast::Ast(int type, const std::string &id)
        : _id {id},
        _left {nullptr},
        _right {nullptr},
        _mid {nullptr},
        _type {type}
{
        typeok(_type);
}

Ast::Ast(int type, Ast *left, int intlit)
        : _left {left},
        _right {nullptr},
        _mid {nullptr},
        _type {type},
        _intlit {intlit}
{
        typeok(_type);
}

Ast::Ast(int type, Ast *left, const std::string &id)
        : _id {id},
        _left {left},
        _right {nullptr},
        _mid {nullptr},
        _type {type}
{
        typeok(_type);
}

Ast *Ast::Left(void) const
{
        return _left;
}

Ast *Ast::Right(void) const
{
        return _right;
}

int Ast::Int(void) const
{
        return _intlit;
}

std::string Ast::Id(void) const
{
        return _id;
}

int Ast::Type(void) const
{
        return _type;
}

std::string Ast::Name(void) const
{
        std::vector<std::string> names {
                "AST_NONE",
                "AST_ADD",
                "AST_SUB",
                "AST_MUL",
                "AST_DIV",
                "AST_INTLIT",
                "AST_IDENT",
                "AST_LVIDENT",
                "AST_ASSIGN",
                "AST_EQ",
                "AST_NE",
                "AST_LT",
                "AST_GT",
                "AST_LE",
                "AST_GE",
                "AST_PRINT",
                "AST_GLUE",
                "AST_IF",
        };

        return names[_type];
}

void astfree(Ast *n)
{
        if (n == nullptr)
                return;
        astfree(n->Left());
        astfree(n->Mid());
        astfree(n->Right());
        delete n;
}

Ast::Ast(int type, Ast *left, Ast *mid, Ast *right, int intlit)
        : _left {left},
        _right {right},
        _mid {mid},
        _type {type},
        _intlit {intlit}
{
        typeok(_type);
}

Ast *Ast::Mid(void) const
{
        return _mid;
}
