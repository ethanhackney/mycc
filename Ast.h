#ifndef AST_H
#define AST_H

#include "Error.h"
#include "Type.h"
#include <string>
#include <vector>

// abstract syntax tree types
enum {
        AST_NONE,       // default type when none given
        AST_ADD,        // add
        AST_SUB,        // subtract
        AST_MUL,        // multiply
        AST_DIV,        // divide
        AST_EQ,         // equal
        AST_NE,         // not equal
        AST_LT,         // less than
        AST_GT,         // greater than
        AST_LE,         // less than or equal
        AST_GE,         // greater than or equal
        AST_INTLIT,     // integer literal
        AST_IDENT,      // identifier
        AST_LVIDENT,    // l-value identifier
        AST_ASSIGN,     // assignment
        AST_PRINT,      // print statement
        AST_GLUE,       // glue node
        AST_IF,         // if statement
        AST_WHILE,      // while statement
        AST_FUNC,       // function
        AST_WIDEN,      // widen data type
        AST_RETURN,     // return statement
        AST_CALL,       // function call
};

// abstract syntax tree
class Ast {
private:
        std::string     _id;            // identifier value
        Ast             *_left;         // left child
        Ast             *_right;        // right child
        Ast             *_mid;          // middle child
        int             _type;          // ast type
        int             _intlit;        // integer literal value
        int             _dtype;         // data type of expression
public:
        // default constructor
        Ast(void);

        // @type:       ast type
        // @dtype:      data type of expression
        // @left:       left child
        // @mid         middle child
        // @right:      right child
        // @intlit:     integer value
        Ast(int type, int dtype, Ast *left, Ast *mid, Ast *right, int intlit);

        // @type        ast type
        // @dtype:      data type of expression
        // @left:       left child
        // @right:      right child
        // @intlit:     integer literal
        Ast(int type, int dtype, Ast *left, Ast *right, int intlit);

        // @type:       ast type
        // @dtype:      data type of expression
        // @left:       left child
        // @right:      right child
        // @id:         identifier
        Ast(int type, int dtype, Ast *left, Ast *right, const std::string &id);

        // @type:       ast type
        // @dtype:      data type of expression
        // @intlit:     integer literal
        Ast(int type, int dtype, int intlit);

        // @type:       ast type
        // @dtype:      data type of expression
        // @id:         identifier
        Ast(int type, int dtype, const std::string &id);

        // @type:       ast type
        // @dtype:      data type of expression
        // @left        left child
        // @intlit:     integer literal
        Ast(int type, int dtype, Ast *left, int intlit);

        // @type:       ast type
        // @dtype:      data type of expression
        // @left:       left child
        // @id:         identifier
        Ast(int type, int dtype, Ast *left, const std::string &id);

        // get left child
        Ast *Left(void) const;

        // get right child
        Ast *Right(void) const;

        // get middle child
        Ast *Mid(void) const;

        // get integer value
        int Int(void) const;

        // get id value
        std::string Id(void) const;

        // get ast type
        int Type(void) const;

        // get ast type name
        std::string Name(void) const;

        int Dtype(void) const;
};

// free ast
//
// @n:  pointer to Ast
extern void astfree(Ast *n);

#endif
