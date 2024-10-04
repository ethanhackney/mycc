#ifndef TYPE_H
#define TYPE_H

#include "Ast.h"
#include "Error.h"
#include <string>

// primitive data types
enum {
        TYPE_NONE,      // nil type
        TYPE_VOID,      // void
        TYPE_CHAR,      // char
        TYPE_INT,       // int
        TYPE_LONG,      // long
        TYPE_VOID_P,    // void pointer
        TYPE_CHAR_P,    // char pointer
        TYPE_INT_P,     // int pointer
        TYPE_LONG_P,    // long pointer
};

// structural types
enum {
        STYPE_VAR,      // variable
        STYPE_FUNC,     // function
};

class CodeGen;

int type_compat(CodeGen& cg, int *left, int *right, int onlyright);

const std::string type_name(int type);

const std::string stype_name(int stype);

int ptr_to(int type);

int val_at(int type);

#endif
