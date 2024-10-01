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
};

// structural types
enum {
        STYPE_VAR,      // variable
        STYPE_FUNC,     // function
};

int type_compat(int *left, int *right, int onlyright);

const std::string type_name(int type);

const std::string stype_name(int stype);

#endif
