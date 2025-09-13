#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/
static void instr_execute_1op()
{
    opr_src.val = vaddr_read(cpu.esp, SREG_SS, data_size / 8);
    cpu.esp += data_size / 8;
    operand_write(&opr_src);
}

make_instr_impl_1op(pop, rm, v)
make_instr_impl_1op(pop, r, v)