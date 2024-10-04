#include "Type.h"
#include "CodeGen.h"

int type_compat(CodeGen& cg, int *left, int *right, int onlyright)
{
        if (*left == *right) {
                *left = 0;
                *right = 0;
                return 1;
        }

        auto leftsize = cg.PrimSize(*left);
        auto rightsize = cg.PrimSize(*right);

        if (leftsize == 0 || rightsize == 0)
                return 0;

        if (leftsize < rightsize) {
                *left = AST_WIDEN;
                *right = 0;
                return 1;
        }

        if (rightsize < leftsize) {
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
        case TYPE_NONE:         return "TYPE_NONE";
        case TYPE_CHAR:         return "TYPE_CHAR";
        case TYPE_INT:          return "TYPE_INT";
        case TYPE_VOID:         return "TYPE_VOID";
        case TYPE_LONG:         return "TYPE_LONG";
        case TYPE_VOID_P:       return "TYPE_VOID_P";
        case TYPE_CHAR_P:       return "TYPE_CHAR_P";
        case TYPE_INT_P:        return "TYPE_INT_P";
        case TYPE_LONG_P:       return "TYPE_LONG_P";
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

int ptr_to(int type)
{
        switch (type) {
        case TYPE_VOID:
                return TYPE_VOID_P;
        case TYPE_CHAR:
                return TYPE_CHAR_P;
        case TYPE_INT:
                return TYPE_INT_P;
        case TYPE_LONG:
                return TYPE_LONG_P;
        default:
                usage("ptr_to(): invalid type: %d", type);
                exit(EXIT_FAILURE);
        }
}

int val_at(int type)
{
        switch (type) {
        case TYPE_VOID_P:
                return TYPE_VOID;
        case TYPE_CHAR_P:
                return TYPE_CHAR;
        case TYPE_INT_P:
                return TYPE_INT;
        case TYPE_LONG_P:
                return TYPE_LONG;
        default:
                usage("val_at(): invalid type %d", type);
                exit(EXIT_FAILURE);
        }
}
