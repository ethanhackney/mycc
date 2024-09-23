#include "CodeGen.h"
#include "Lexer.h"
#include "Parser.h"
#include <cstdio>

int main(int argc, char **argv)
{
        Lexer l {"input"};
        CodeGen cg {"out.s"};
        Parser p {l, cg};

        auto n = p.ParseCompound();
        cg.GenPre();
        cg.GenAst(n, (size_t)0, AST_NONE);
        cg.GenPost();
        astfree(n);
}
