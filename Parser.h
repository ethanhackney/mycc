#ifndef PARSER_H
#define PARSER_H

#include "Ast.h"
#include "CodeGen.h"
#include "Error.h"
#include "Lexer.h"
#include "Type.h"
#include <string>

extern std::string func_id;

// parser
class Parser {
private:
        CodeGen         &_cg;   // reference to code generator
        Lexer           &_lex;  // reference to lexical analyzer

        // parse a print statement
        Ast *parsePrint(void);
        // parse a variable declaration statement
        void parseVarDecl(int type, const std::string& id);
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
        // parse return statement
        Ast *parseRet(void);
        // parse function call
        Ast *parseCall(const std::string& id);
        // parse prefix
        Ast *parsePrefix(void);
public:
        // @lex:        reference to lexical analyzer
        // @cg:         reference to code generator
        Parser(Lexer &lex, CodeGen &cg);

        // parse a compound statement
        Ast *ParseCompound(void);

        // parse a function declaration
        Ast *ParseFuncDecl(int type, const std::string& id);

        // parse global declarations
        void ParseDecls(void);
};

#endif
