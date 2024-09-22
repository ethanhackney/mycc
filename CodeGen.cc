#include "CodeGen.h"

CodeGen::CodeGen(const std::string &path)
        : _path {path},
        _stk {},
        _tab {},
        _fp {fopen(path.c_str(), "w")}
{
        if (_fp == nullptr)
                error("could not open %s", path.c_str());
}

CodeGen::~CodeGen()
{
        if (fclose(_fp) < 0)
                error("could not close %s", _path.c_str());
}

void CodeGen::GenPre(void)
{
        fputs("\t.text\n"
	      ".LC0:\n"
	      "\t.string\t\"%d\\n\"\n"
	      "printint:\n"
	      "\tpushq\t%rbp\n"
	      "\tmovq\t%rsp, %rbp\n"
	      "\tsubq\t$16, %rsp\n"
	      "\tmovl\t%edi, -4(%rbp)\n"
	      "\tmovl\t-4(%rbp), %eax\n"
	      "\tmovl\t%eax, %esi\n"
	      "\tleaq	.LC0(%rip), %rdi\n"
	      "\tmovl	$0, %eax\n"
	      "\tcall	printf@PLT\n"
	      "\tnop\n"
	      "\tleave\n"
	      "\tret\n"
	      "\n"
	      "\t.globl\tmain\n"
	      "\t.type\tmain, @function\n"
	      "main:\n" "\tpushq\t%rbp\n" "\tmovq	%rsp, %rbp\n", _fp);
}

void CodeGen::GenPost(void)
{
        fputs("\tmovl	$0, %eax\n" "\tpopq	%rbp\n" "\tret\n", _fp);
}

void CodeGen::GenPrintInt(size_t r)
{
        fprintf(_fp, "\tmovq\t%s, %%rdi\n", _stk.Name(r));
        fprintf(_fp, "\tcall\tprintint\n");
        _stk.Put(r);
}

void CodeGen::GenGlo(const std::string &id)
{
        fprintf(_fp, "\t.comm\t%s,8,8\n", id.c_str());
}

size_t CodeGen::GenAst(Ast *n, size_t r)
{
        size_t left;
        size_t right;

        if (n->Left())
                left = GenAst(n->Left(), (size_t)-1);
        if (n->Right())
                right = GenAst(n->Right(), left);

        switch (n->Type()) {
        case AST_ADD:
                return add(left, right);
        case AST_SUB:
                return sub(left, right);
        case AST_MUL:
                return mul(left, right);
        case AST_DIV:
                return div(left, right);
        case AST_EQ:
                return eq(left, right);
        case AST_NE:
                return ne(left, right);
        case AST_LT:
                return lt(left, right);
        case AST_GT:
                return gt(left, right);
        case AST_LE:
                return le(left, right);
        case AST_GE:
                return ge(left, right);
        case AST_INTLIT:
                return movInt(n->Int());
        case AST_IDENT:
                return movGlo(n->Id());
        case AST_LVIDENT:
                return strGlo(r, n->Id());
        case AST_ASSIGN:
                return right;
        default:
                usage("invalid token type: %s", n->Name().c_str());
                exit(EXIT_FAILURE);
        }
}

size_t CodeGen::add(size_t i, size_t j)
{
        fprintf(_fp, "\taddq\t%s, %s\n", _stk.Name(i), _stk.Name(j));
        _stk.Put(i);
        return j;
}

size_t CodeGen::sub(size_t i, size_t j)
{
        fprintf(_fp, "\tsubq\t%s, %s\n", _stk.Name(j), _stk.Name(i));
        _stk.Put(j);
        return i;
}

size_t CodeGen::mul(size_t i, size_t j)
{
        fprintf(_fp, "\timulq\t%s, %s\n", _stk.Name(i), _stk.Name(j));
        _stk.Put(i);
        return j;
}

size_t CodeGen::div(size_t i, size_t j)
{
        fprintf(_fp, "\tmovq\t%s, %%rax\n", _stk.Name(i));
        fprintf(_fp, "\tcqo\n");
        fprintf(_fp, "\tidivq\t%s\n", _stk.Name(j));
        fprintf(_fp, "\tmovq\t%%rax, %s\n", _stk.Name(i));
        _stk.Put(j);
        return i;
}

size_t CodeGen::movInt(int v)
{
        size_t r = _stk.Get();
        fprintf(_fp, "\tmovq\t$%d, %s\n", v, _stk.Name(r));
        return r;
}

size_t CodeGen::movGlo(const std::string &id)
{
        size_t r = _stk.Get();
        fprintf(_fp, "\tmovq\t%s(%%rip), %s\n", id.c_str(), _stk.Name(r));
        return r;
}

size_t CodeGen::strGlo(size_t r, const std::string &id)
{
        fprintf(_fp, "\tmovq\t%s, %s(%%rip)\n", _stk.Name(r), id.c_str());
        return r;
}

Sym *CodeGen::GetGlo(const std::string &id)
{
        return _tab.Get(id);
}

void CodeGen::Free(void)
{
        _stk.Free();
}

void CodeGen::SetGlo(const std::string &id)
{
        _tab.Set(id, new Sym{id});
}

size_t CodeGen::cmp(size_t i, size_t j, const std::string &how)
{
        std::string breg = std::string{_stk.Name(j)} + "b";
        fprintf(_fp, "\tcmpq\t%s, %s\n", _stk.Name(j), _stk.Name(i));
        fprintf(_fp, "\t%s\t%s\n", how.c_str(), breg.c_str());
        fprintf(_fp, "\tandq\t$255, %s\n", _stk.Name(j));
        _stk.Put(i);
        return j;
}

size_t CodeGen::eq(size_t i, size_t j)
{
        return cmp(i, j, "sete");
}

size_t CodeGen::ne(size_t i, size_t j)
{
        return cmp(i, j, "setne");
}

size_t CodeGen::lt(size_t i, size_t j)
{
        return cmp(i, j, "setl");
}

size_t CodeGen::gt(size_t i, size_t j)
{
        return cmp(i, j, "setg");
}

size_t CodeGen::le(size_t i, size_t j)
{
        return cmp(i, j, "setle");
}

size_t CodeGen::ge(size_t i, size_t j)
{
        return cmp(i, j, "setge");
}
