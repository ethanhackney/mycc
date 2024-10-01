#include "Sym.h"

Sym::Sym(int prim, int stype, const std::string &name)
        : _name {name},
        _prim {prim},
        _stype {stype}
{
        switch (_prim) {
        case TYPE_NONE:
        case TYPE_VOID:
        case TYPE_CHAR:
        case TYPE_INT:
                break;
        default:
                usage("invalid primitive data type: %d", _prim);
        }

        switch (_stype) {
        case STYPE_VAR:
        case STYPE_FUNC:
                break;
        default:
                usage("invalid structural type: %d", _stype);
        }
};

std::string Sym::Name(void) const
{
        return _name;
}

int Sym::Prim(void) const
{
        return _prim;
}

int Sym::Stype(void) const
{
        return _stype;
}
