#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op()
{
    opr_src.data_size = data_size;
    cpu.esp -= data_size / 8;
    opr_src.addr = cpu.esp;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    operand_write(&opr_src);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, rm, v)

make_instr_func(push_cs)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    opr_src.val = SREG_CS;
    operand_write(&opr_src);
    return 1;
}

make_instr_func(push_ss)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    opr_src.val = SREG_SS;
    operand_write(&opr_src);
    return 1;
}

make_instr_func(push_ds)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    opr_src.val = SREG_DS;
    operand_write(&opr_src);
    return 1;
}

make_instr_func(push_es)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    opr_src.val = SREG_ES;
    operand_write(&opr_src);
    return 1;
}

make_instr_func(push_fs)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    opr_src.val = SREG_FS;
    operand_write(&opr_src);
    return 1;
}

make_instr_func(push_gs)
{
    opr_src.data_size = 16;
    cpu.esp -= 2;
    opr_src.addr = cpu.esp;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    opr_src.val = SREG_GS;
    operand_write(&opr_src);
    return 1;
}