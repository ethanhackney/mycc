#include "CodeGen.h"
#include "Parser.h"

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

int CodeGen::GetLabel(void)
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
        Free();
        fputs("\t.text\n", _fp);
}

void CodeGen::GenPost(const std::string& id)
{
        auto s = _tab.Get(id);
        label(s->End());
        fputs("\tpopq   %rbp\n"
              "\tret\n",
              _fp);
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
        int size = PrimSize(s->Prim());

        fprintf(_fp, "\t.data\n");
        fprintf(_fp, "\t.globl\t%s\n", id.c_str());

        switch (size) {
        case 1:
                fprintf(_fp, "%s:\t.byte\t0\n", id.c_str());
                break;
        case 4:
                fprintf(_fp, "%s:\t.long\t0\n", id.c_str());
                break;
        case 8:
                fprintf(_fp, "%s:\t.quad\t0\n", id.c_str());
                break;
        default:
                usage("unknown type size: %d", size);
        }
}

size_t CodeGen::genIfAst(Ast *n)
{
        int falseid;
        int endid;

        falseid = GetLabel();
        if (n->Right())
                endid = GetLabel();

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
                funcPost(n->Id());
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
                if (n->Rval() || parentop == AST_DEREF)
                        return movGlo(n->Id());
                return NIL_REG;
        case AST_ASSIGN:
                switch (n->Right()->Type()) {
                case AST_IDENT:
                        return strGlo(left, n->Right()->Id());
                case AST_DEREF:
                        return strDeref(left, right, n->Right()->Dtype());
                default:
                        usage("bad operation: %s", n->Name().c_str());
                        exit(EXIT_FAILURE);
                }
        case AST_WIDEN:
                return widen(left, n->Left()->Dtype(), n->Dtype());
        case AST_RETURN:
                ret(left, func_id);
                return NIL_REG;
        case AST_CALL:
                return call(left, n->Id());
        case AST_ADDR:
                return addr(n->Id());
        case AST_DEREF:
                if (n->Rval())
                        return deref(left, n->Left()->Dtype());
                return left;
        case AST_SCALE:
                switch (n->Int()) {
                case 2:
                        return shl_const(left, 1);
                case 4:
                        return shl_const(left, 2);
                case 8:
                        return shl_const(left, 3);
                default:
                        right = movInt(n->Int());
                        return mul(left, right);
                }
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

        switch (s->Prim()) {
        case TYPE_CHAR:
                fprintf(_fp, "movzbq\t%s(%%rip), %s\n", id.c_str(),
                                _stk.Name(r));
                break;
        case TYPE_INT:
                /* NOTE: this is the assembly line that was
                 * having problems: it use to be movzbl but the
                 * assembler didn't like that, but it likes this
                 * and i dont know why
                 */
                fprintf(_fp, "movzbq\t%s(%%rip), %s\n", id.c_str(),
                                _stk.Name(r));
                break;
        case TYPE_LONG:
        case TYPE_CHAR_P:
        case TYPE_INT_P:
        case TYPE_LONG_P:
                fprintf(_fp, "\tmovq\t%s(%%rip), %s\n", id.c_str(),
                                _stk.Name(r));
                break;
        default:
                usage("invalid data type: %s", type_name(s->Prim()));
                exit(1);
        }

        return r;
}

size_t CodeGen::strGlo(size_t r, const std::string &id)
{
        std::string reg;
        auto s = _tab.Get(id);

        switch (s->Prim()) {
        case TYPE_CHAR:
                reg = std::string{_stk.Name(r)} + "b";
                fprintf(_fp, "\tmovb\t%s, %s(%%rip)\n",
                                reg.c_str(), id.c_str());
                break;;
        case TYPE_INT:
                reg = std::string{_stk.Name(r)} + "d";
                fprintf(_fp, "movl\t%s, %s(%%rip)\n",
                                reg.c_str(), id.c_str());
                break;
        case TYPE_LONG:
        case TYPE_CHAR_P:
        case TYPE_INT_P:
        case TYPE_LONG_P:
                fprintf(_fp, "movq\t%s, %s(%%rip)\n",
                                _stk.Name(r), id.c_str());
                break;
        default:
                usage("bad primitive: %s", type_name(s->Prim()));
                exit(1);
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

void CodeGen::SetGlo(int prim, int stype, int end, const std::string &id)
{
        _tab.Set(id, new Sym{prim, stype, end, id});
}

void CodeGen::SetGlo(int prim, int stype, int end, const std::string &id, int size)
{
        _tab.Set(id, new Sym{prim, stype, end, id, size});
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
                usage("cmp_and_jmp: bad ast type",
                                Ast{type, TYPE_NONE, 0}.Name().c_str());
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
                usage("cmp_and_set: bad ast type: %s\n",
                                Ast{type, TYPE_NONE, 0}.Name().c_str());
        }

        fprintf(_fp, "\tcmpq\t%s, %s\n", _stk.Name(j), _stk.Name(i));
        fprintf(_fp, "\t%s\t%s\n", sets[type - AST_EQ], b.c_str());
        fprintf(_fp, "\tmovzbq\t%s, %s\n", b.c_str(), _stk.Name(j));
        _stk.Put(i);
        return j;
}

size_t CodeGen::genWhile(Ast *n)
{
        auto start = GetLabel();
        auto end = GetLabel();
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
CodeGen::funcPost(const std::string& id)
{
        GenPost(id);
}

size_t CodeGen::widen(size_t r, int oldtype, int newtype)
{
        return r;
}

size_t CodeGen::PrimSize(int prim)
{
        static int sizes[] = {
                0, 0, 1, 4, 8, 8, 8, 8,
        };

        if (prim < 0 || prim >= (int)(sizeof(sizes) / sizeof(*sizes)))
                usage("invalid primitive type: %s", type_name(prim));

        return sizes[prim];
}

void CodeGen::ret(size_t r, const std::string& id)
{
        std::string reg;
        auto s = _tab.Get(id);

        switch (s->Prim()) {
        case TYPE_CHAR:
                reg = std::string{_stk.Name(r)} + "b";
                fprintf(_fp, "\tmovzbl\t%s, %%eax\n", reg.c_str());
                break;
        case TYPE_INT:
                reg = std::string{_stk.Name(r)} + "d";
                fprintf(_fp, "\tmovl\t%s, %%eax\n", reg.c_str());
                break;
        case TYPE_LONG:
                fprintf(_fp, "\tmovq\t%s, %%rax\n", _stk.Name(r));
                break;
        default:
                usage("bad type: %s", type_name(s->Prim()));
                exit(1);
        }

        jmp(s->End());
}

size_t CodeGen::call(size_t r, const std::string& id)
{
        size_t out = _stk.Get();
        fprintf(_fp, "\tmovq\t%s, %%rdi\n", _stk.Name(r));
        fprintf(_fp, "\tcall\t%s\n", id.c_str());
        fprintf(_fp, "\tmovq\t%%rax, %s\n", _stk.Name(out));
        _stk.Put(r);
        return out;
}

size_t CodeGen::addr(const std::string& id)
{
        auto r = _stk.Get();
        fprintf(_fp, "\tleaq\t%s(%%rip), %s\n", id.c_str(), _stk.Name(r));
        return r;
}

size_t CodeGen::deref(size_t r, int datatype)
{
        switch (datatype) {
        case TYPE_CHAR_P:
                fprintf(_fp, "\tmovzbq\t(%s), %s\n", _stk.Name(r),
                                _stk.Name(r));
                break;
        case TYPE_INT_P:
                fprintf(_fp, "\tmovq\t(%s), %s\n", _stk.Name(r),
                                _stk.Name(r));
                break;
        case TYPE_LONG_P:
                fprintf(_fp, "\tmovq\t(%s), %s\n", _stk.Name(r),
                                _stk.Name(r));
                break;
        }
        return r;
}

size_t CodeGen::shl_const(size_t r, int val)
{
        fprintf(_fp, "\tsalq\t$%d, %s\n", val, _stk.Name(r));
        return r;
}

size_t CodeGen::strDeref(size_t r1, size_t r2, int type)
{
        std::string reg;

        switch (type) {
        case TYPE_CHAR:
                reg = std::string{_stk.Name(r1)} + "b";
                fprintf(_fp, "\tmovb\t%s, (%s)\n",
                        reg.c_str(), _stk.Name(r2));
                break;
        case TYPE_INT:
        case TYPE_LONG:
                fprintf(_fp, "\tmovq\t%s, (%s)\n",
                        _stk.Name(r1), _stk.Name(r2));
                break;
        default:
                usage("bad deref");
        }

        return r1;
}
