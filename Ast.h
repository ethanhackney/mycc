#ifndef AST_H
#define AST_H

#include "Error.h"
#include <string>
#include <vector>

// abstract syntax tree types
enum {
        AST_NONE,       // default type when none given
        AST_ADD,        // add
        AST_SUB,        // subtract
        AST_MUL,        // multiply
        AST_DIV,        // divide
        AST_INTLIT,     // integer literal
        AST_IDENT,      // identifier
        AST_LVIDENT,    // l-value identifier
        AST_ASSIGN,     // assignment
};

// abstract syntax tree
class Ast {
private:
        std::string     _id;            // identifier value
        Ast             *_left;         // left child
        Ast             *_right;        // right child
        int             _type;          // ast type
        int             _intlit;        // integer literal value
public:
        // default constructor
        Ast(void);

        // @type        ast type
        // @left:       left child
        // @right:      right child
        // @intlit:     integer literal
        Ast(int type, Ast *left, Ast *right, int intlit);

        // @type:       ast type
        // @left:       left child
        // @right:      right child
        // @id:         identifier
        Ast(int type, Ast *left, Ast *right, const std::string &id);

        // @type:       ast type
        // @intlit:     integer literal
        Ast(int type, int intlit);

        // @type:       ast type
        // @id:         identifier
        Ast(int type, const std::string &id);

        // @type:       ast type
        // @left        left child
        // @intlit:     integer literal
        Ast(int type, Ast *left, int intlit);

        // @type:       ast type
        // @left:       left child
        // @id:         identifier
        Ast(int type, Ast *left, const std::string &id);

        // get left child
        Ast *Left(void) const;

        // get right child
        Ast *Right(void) const;

        // get integer value
        int Int(void) const;

        // get id value
        std::string Id(void) const;

        // get ast type
        int Type(void) const;

        // get ast type name
        std::string Name(void) const;
};

// free ast
//
// @n:  pointer to Ast
extern void astfree(Ast *n);

#endif
