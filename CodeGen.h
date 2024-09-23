#ifndef CODEGEN_H
#define CODEGEN_H

#include "Ast.h"
#include "Error.h"
#include "RegStk.h"
#include "SymTab.h"
#include <cstdio>
#include <string>

// code generator
class CodeGen {
private:
        std::string     _path;  // path name of output file
        RegStk          _stk;   // register stack
        SymTab          _tab;   // symbol table
        FILE            *_fp;   // output file
        int             _id;    // id of next available label

        // generate instructions for global variable
        void genGlo(const std::string &id);
        // generate add instruction
        size_t add(size_t i, size_t j);
        // generate sub instruction
        size_t sub(size_t i, size_t j);
        // generate mul instruction
        size_t mul(size_t i, size_t j);
        // generate div instruction
        size_t div(size_t i, size_t j);
        // generate mov for integer
        size_t movInt(int v);
        // generate mov for global variable
        size_t movGlo(const std::string &id);
        // generate store for global variable
        size_t strGlo(size_t r, const std::string &id);
        // generate instructions for comparison
        size_t cmp(size_t i, size_t j, const std::string &how);
        // generate instructions for equality test
        size_t eq(size_t i, size_t j);
        // generate instructions for not equal test
        size_t ne(size_t i, size_t j);
        // generate instructions for less than test
        size_t lt(size_t i, size_t j);
        // generate instructions for greater than test
        size_t gt(size_t i, size_t j);
        // generate instructions for less than or equal test
        size_t le(size_t i, size_t j);
        // generate instructions for greater than or equal test
        size_t ge(size_t i, size_t j);
        // generate instructions for if statement
        size_t genIfAst(Ast *n);
        // get a new label
        int getLabel(void);
        // generate jump instruction
        void jmp(int label);
        // generate label instruction
        void label(int label);
        // generate compare and jump
        size_t cmp_and_jmp(int type, size_t i, size_t j, int label);
        // generate compare and set
        size_t cmp_and_set(int type, size_t i, size_t j);
public:
        // @path:       path name of output file
        CodeGen(const std::string &path);

        ~CodeGen();

        // generate preamble
        void GenPre(void);

        // generate postamble
        void GenPost(void);

        // generate code to print int
        void GenPrintInt(size_t r);

        // generate code for AST
        size_t GenAst(Ast *n, size_t r, int parentop);

        // generate code for global variable
        void GenGlo(const std::string &id);

        // get symbol
        Sym *GetGlo(const std::string &id);

        // free all registers
        void Free(void);

        // get symbol
        void SetGlo(const std::string &id);
};

#endif
