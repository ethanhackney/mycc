#include "CodeGen.h"
#include "Lexer.h"
#include "Parser.h"
#include <cstdio>

int main(int argc, char **argv)
{
        Lexer l {"input"};
        CodeGen cg {"out.s"};
        Parser p {l, cg};

        cg.GenPre();
        for (;;) {
                p.ParseStmt();
                if (l.Curr().Type() == TOK_EOF)
                        break;
        }
        cg.GenPost();
}
