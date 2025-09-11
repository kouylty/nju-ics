#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op()
{
    opr_src.data_size = data_size;
    cpu.esp -= data_size / 8;
    opr_src.addr = cpu.esp;
    vaddr_write(opr_src.addr, SREG_SS, data_size / 8, opr_src.val);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, rm, v)
/*
make_instr_func(push_cs)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
#ifdef IA32_SEG
    opr_src.val = cpu.cs.val;
    vaddr_write(opr_src.addr, SREG_SS, 2, opr_src.val);
#endif
    return 1;
}

make_instr_func(push_ss)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.val = SREG_SS;
    vaddr_write(opr_src.addr, SREG_SS, 2, opr_src.val);
    return 1;
}

make_instr_func(push_ds)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.val = SREG_DS;
    vaddr_write(opr_src.addr, SREG_SS, 2, opr_src.val);
    return 1;
}

make_instr_func(push_es)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.val = SREG_ES;
    vaddr_write(opr_src.addr, SREG_SS, 2, opr_src.val);
    return 1;
}

make_instr_func(push_fs)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.val = SREG_FS;
    vaddr_write(opr_src.addr, SREG_SS, 2, opr_src.val);
    return 1;
}

make_instr_func(push_gs)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.val = SREG_GS;
    vaddr_write(opr_src.addr, SREG_SS, 2, opr_src.val);
    return 1;
}
*/