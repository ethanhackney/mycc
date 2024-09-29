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

        cg.GenPre();
        for (;;) {
                auto n = p.ParseFuncDecl();
                cg.GenAst(n, NIL_REG, AST_NONE);
                astfree(n);
                if (l.Curr().Type() == TOK_EOF)
                        break;
        }
}
