#include "RegStk.h"

RegStk::RegStk(void)
{
        Free();
}

size_t RegStk::Get(void)
{
        if (_stk.empty())
                usage("no more registers left");

        auto r = _stk.back();
        _stk.pop_back();
        return r;
}

void RegStk::Put(size_t r)
{
        if (r > _regs.size())
                usage("freeing invalid register: %zu", r);

        if (_stk.size() == _regs.size())
                usage("register stack already full");

        _stk.push_back(r);
}

const char *RegStk::Name(size_t r)
{
        if (r > _regs.size())
                usage("invalid register: %zu", r);

        return _regs[r];
}

void RegStk::Free(void)
{
        for (size_t i = 0; i < _regs.size(); i++)
                _stk.push_back(i);
}
