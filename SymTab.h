#ifndef SYMTAB_H
#define SYMTAB_H

#include "Error.h"
#include "Sym.h"
#include <string>
#include <unordered_map>

// symbol table
class SymTab {
private:
        std::unordered_map<std::string,Sym*>    _tab;   // symbol table
public:
        ~SymTab();
        // add symbol to table
        void Set(const std::string &name, Sym *sym);

        // get symbol from table
        Sym *Get(const std::string &name);
};

#endif
