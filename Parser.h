#ifndef PARSER_H
#define PARSER_H

#include "Ast.h"
#include "CodeGen.h"
#include "Error.h"
#include "Lexer.h"

// parser
class Parser {
private:
        CodeGen &_cg;   // reference to code generator
        Lexer   &_lex;  // reference to lexical analyzer

        // parse a print statement
        Ast *parsePrint(void);
        // parse a variable declaration statement
        void parseVarDecl(void);
        // parse assignment statement
        Ast *parseAssign(void);
        // parse expression
        Ast *parseExpr(int ptp);
        // parse primary
        Ast *parsePrimary(void);
        // parse if statement
        Ast *parseIf(void);
        // parse while statement
        Ast *parseWhile(void);
        // parse for statement
        Ast *parseFor(void);
        // parse single statement
        Ast *parseSingle(void);
public:
        // @lex:        reference to lexical analyzer
        // @cg:         reference to code generator
        Parser(Lexer &lex, CodeGen &cg);

        // parse a compound statement
        Ast *ParseCompound(void);
};

#endif
