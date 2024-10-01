#include "Type.h"

int type_compat(int *left, int *right, int onlyright)
{
        if (*left == TYPE_VOID || *right == TYPE_VOID)
                return 0;

        if (*left == *right) {
                *left = 0;
                *right = 0;
                return 1;
        }

        if (*left == TYPE_CHAR && *right == TYPE_INT) {
                *left = AST_WIDEN;
                *right = 0;
                return 1;
        }

        if (*left == TYPE_INT && *right == TYPE_CHAR) {
                if (onlyright)
                        return 0;
                *left = 0;
                *right = AST_WIDEN;
                return 1;
        }

        *left = 0;
        *right = 0;
        return 1;
}

const std::string type_name(int type)
{
        switch (type) {
        case TYPE_NONE: return "TYPE_NONE";
        case TYPE_CHAR: return "TYPE_CHAR";
        case TYPE_INT:  return "TYPE_INT";
        case TYPE_VOID: return "TYPE_VOID";
        default:
                usage("invalid type name: %d", type);
                exit(0);
        }
}

const std::string stype_name(int stype)
{
        switch (stype) {
        case STYPE_VAR:  return "STYPE_VAR";
        case STYPE_FUNC: return "STYPE_FUNC";
        default:
                usage("invalid stype name: %d", stype);
                exit(0);
        }
}
