#include "CodeGen.h"
#include "Lexer.h"
#include "Parser.h"
#include <cstdio>

int main(int argc, char **argv)
{
        if (argc != 2) {
                fprintf(stderr, "a.out input\n");
                exit(1);
        }

        Lexer l {argv[1]};
        CodeGen cg {"out.s"};
        Parser p {l, cg};

        auto n = p.ParseCompound();
        cg.GenPre();
        cg.GenAst(n, (size_t)0, AST_NONE);
        cg.GenPost();
        astfree(n);
}
