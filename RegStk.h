#ifndef REGSTK_H
#define REGSTK_H

#include "Error.h"
#include <vector>

// register stack
class RegStk {
private:
        std::vector<const char *>       _regs { // register names
                "%r8",
                "%r9",
                "%r10",
                "%r11",
        };
        std::vector<size_t>             _stk;   // register stack
public:
        // default constructor
        RegStk(void);

        // get a register from stack
        size_t Get(void);

        // put register back onto stack
        void Put(size_t r);

        // get register name
        const char *Name(size_t r);

        // add all registers back onto stack
        void Free(void);

        size_t Size(void) const;
};

#endif
