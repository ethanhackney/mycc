#ifndef SYM_H
#define SYM_H

#include "Error.h"
#include "Type.h"
#include <string>

// symbol
class Sym {
private:
        std::string     _name;  // symbol name
        int             _prim;  // primitive type
        int             _stype; // structural type
public:
        // @name:       symbol name
        // @prim:       primitive type
        // @stype:      structural type
        Sym(int prim, int stype, const std::string &name);

        // get symbol name
        std::string Name(void) const;

        int Prim(void) const;

        int Stype(void) const;
};

#endif
