#include "SymTab.h"

void SymTab::Set(const std::string &name, Sym *sym)
{
        auto p = _tab.find(name);

        if (p != _tab.end())
                usage("%s already in symbol table", name.c_str());

        _tab[name] = sym;
}

Sym *SymTab::Get(const std::string &name)
{
        auto p = _tab.find(name);

        if (p == _tab.end())
                usage("%s not in symbol table", name.c_str());

        return p->second;
}

SymTab::~SymTab()
{
        for (auto p : _tab)
                delete p.second;
}
