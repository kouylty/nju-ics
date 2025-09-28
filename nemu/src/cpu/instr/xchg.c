#include "cpu/instr.h"
/*
Put the implementations of `xchg' instructions here.
*/
static void instr_execute_2op() 
{
	operand_read(&opr_src);
	operand_read(&opr_dest);
	int temp = opr_src.val;
	opr_src.val = opr_dest.val;
	opr_dest.val = temp;
	operand_write(&opr_src);
	operand_write(&opr_dest);
}

make_instr_impl_2op(xchg, r, rm, b)
make_instr_impl_2op(xchg, r, rm, v)
make_instr_impl_2op(xchg, rm, r, b)
make_instr_impl_2op(xchg, rm, r, v)