#include "CodeGen.h"

CodeGen::CodeGen(const std::string &path)
        : _path {path},
        _stk {},
        _tab {},
        _fp {fopen(path.c_str(), "w")},
        _id {1}
{
        if (_fp == nullptr)
                error("could not open %s", path.c_str());
}

int CodeGen::getLabel(void)
{
        return _id++;
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
              "\n", _fp);
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
        auto s = _tab.Get(id);

        if (s->Prim() == TYPE_INT)
                fprintf(_fp, "\t.comm\t%s,8,8\n", id.c_str());
        else
                fprintf(_fp, "\t.comm\t%s,1,1\n", id.c_str());
}

size_t CodeGen::genIfAst(Ast *n)
{
        int falseid;
        int endid;

        falseid = getLabel();
        if (n->Right())
                endid = getLabel();

        GenAst(n->Left(), (size_t)falseid, n->Type());
        Free();

        GenAst(n->Mid(), NIL_REG, n->Type());
        Free();

        if (n->Right())
                jmp(endid);

        label(falseid);

        if (n->Right()) {
                GenAst(n->Right(), NIL_REG, n->Type());
                Free();
                label(endid);
        }

        return NIL_REG;
}

size_t CodeGen::GenAst(Ast *n, size_t r, int parentop)
{
        size_t left;
        size_t right;

        switch (n->Type()) {
        case AST_IF:
                return genIfAst(n);
        case AST_WHILE:
                return genWhile(n);
        case AST_GLUE:
                GenAst(n->Left(), NIL_REG, n->Type());
                Free();
                GenAst(n->Right(), NIL_REG, n->Type());
                Free();
                return NIL_REG;
        case AST_FUNC:
                funcPre(n->Id());
                GenAst(n->Left(), NIL_REG, n->Type());
                funcPost();
                return NIL_REG;
        }

        if (n->Left())
                left = GenAst(n->Left(), NIL_REG, n->Type());
        if (n->Right())
                right = GenAst(n->Right(), left, n->Type());

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
        case AST_NE:
        case AST_LT:
        case AST_GT:
        case AST_LE:
        case AST_GE:
                if (parentop == AST_IF || parentop == AST_WHILE)
                        return cmp_and_jmp(n->Type(), left, right, r);
                return cmp_and_set(n->Type(), left, right);
        case AST_INTLIT:
                return movInt(n->Int());
        case AST_IDENT:
                return movGlo(n->Id());
        case AST_LVIDENT:
                return strGlo(r, n->Id());
        case AST_ASSIGN:
                return right;
        case AST_PRINT:
                GenPrintInt(left);
                Free();
                return NIL_REG;
        case AST_WIDEN:
                return widen(left, n->Left()->Dtype(), n->Dtype());
        default:
                usage("invalid ast type: %s", n->Name().c_str());
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
        auto s = _tab.Get(id);

        if (s->Prim() == TYPE_INT)
                fprintf(_fp, "\tmovq\t%s(%%rip), %s\n", id.c_str(), _stk.Name(r));
        else
                fprintf(_fp, "\tmovzbq\t%s(%%rip), %s\n", id.c_str(), _stk.Name(r));

        return r;
}

size_t CodeGen::strGlo(size_t r, const std::string &id)
{
        auto s = _tab.Get(id);

        if (s->Prim() == TYPE_INT) {
                fprintf(_fp, "\tmovq\t%s, %s(%%rip)\n",
                                _stk.Name(r), id.c_str());
        } else {
                auto reg = std::string{_stk.Name(r)} + "b";
                fprintf(_fp, "\tmovb\t%s, %s(%%rip)\n",
                                reg.c_str(), id.c_str());
        }

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

void CodeGen::SetGlo(int prim, int stype, const std::string &id)
{
        _tab.Set(id, new Sym{prim, stype, id});
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

void CodeGen::jmp(int label)
{
        fprintf(_fp, "\tjmp\tL%d\n", label);
}

void CodeGen::label(int l)
{
        fprintf(_fp, "L%d:\n", l);
}

size_t CodeGen::cmp_and_jmp(int type, size_t i, size_t j, int label)
{
        static const char *jmps[] = {
                "jne",
                "je",
                "jge",
                "jle",
                "jg",
                "jl",
        };

        if (type < AST_EQ || type > AST_GE) {
                puts("here");
                usage("cmp_and_jmp: bad ast type",
                                Ast{type, TYPE_NONE, 0}.Name().c_str());
                puts("here");
        }

        fprintf(_fp, "\tcmpq\t%s, %s\n", _stk.Name(j), _stk.Name(i));
        fprintf(_fp, "\t%s\tL%d\n", jmps[type - AST_EQ], label);
        Free();
        return NIL_REG;
}

size_t CodeGen::cmp_and_set(int type, size_t i, size_t j)
{
        static const char *sets[] = {
                "sete",
                "setne",
                "setl",
                "setg",
                "setle",
                "setge",
        };
        std::string b = std::string{_stk.Name(j)} + "b";

        if (type < AST_EQ || type > AST_GE) {
                puts("here");
                usage("cmp_and_set: bad ast type: %s\n",
                                Ast{type, TYPE_NONE, 0}.Name().c_str());
                puts("here");
        }

        fprintf(_fp, "\tcmpq\t%s, %s\n", _stk.Name(j), _stk.Name(i));
        fprintf(_fp, "\t%s\t%s\n", sets[type - AST_EQ], b.c_str());
        fprintf(_fp, "\tmovzbq\t%s, %s\n", b.c_str(), _stk.Name(j));
        _stk.Put(i);
        return j;
}

size_t CodeGen::genWhile(Ast *n)
{
        auto start = getLabel();
        auto end = getLabel();
        label(start);
        GenAst(n->Left(), end, n->Type());
        Free();
        GenAst(n->Right(), NIL_REG, n->Type());
        Free();
        jmp(start);
        label(end);
        return NIL_REG;
}

void CodeGen::funcPre(const std::string &id)
{
        fprintf(_fp,
                "\t.text\n"
                "\t.globl\t%s\n"
                "\t.type\t%s, @function\n"
                "%s:\n"
                "\tpushq\t%%rbp\n"
                "\tmovq\t%%rsp, %%rbp\n",
                id.c_str(),
                id.c_str(),
                id.c_str());
}

void
CodeGen::funcPost(void)
{
        GenPost();
}

size_t CodeGen::widen(size_t r, int oldtype, int newtype)
{
        return r;
}
