#include "Sym.h"

Sym::Sym(const std::string &name)
        : _name {name}
{}

std::string Sym::Name(void) const
{
        return _name;
}
