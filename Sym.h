#ifndef SYM_H
#define SYM_H

#include <string>

// symbol
class Sym {
private:
        std::string     _name;  // symbol name
public:
        // @name:       symbol name
        Sym(const std::string &name);

        // get symbol name
        std::string Name(void) const;
};

#endif
