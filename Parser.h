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
        void parse_print(void);
        // parse a variable declaration statement
        void parse_var_decl(void);
        // parse assignment statement
        void parse_assign(void);
        // parse expression
        Ast *parse_expr(int ptp);
        // parse primary
        Ast *parse_primary(void);
public:
        // @lex:        reference to lexical analyzer
        // @cg:         reference to code generator
        Parser(Lexer &lex, CodeGen &cg);

        // parse a statement
        void ParseStmt(void);
};

#endif
