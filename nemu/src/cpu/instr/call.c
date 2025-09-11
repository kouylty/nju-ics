#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/

make_instr_func(call_near)
{
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, 4, cpu.eip);
	OPERAND rel;
	rel.type = OPR_IMM;
	rel.sreg = SREG_CS;
	rel.data_size = data_size;
	rel.addr = eip + 1;
	operand_read(&rel);
	int offset = sign_ext(rel.val, data_size);
	// thank Ting Xu from CS'17 for finding this bug
	print_asm_1("call", "", 1 + data_size / 8, &rel);
	cpu.eip += offset;
	return 1 + data_size / 8;
}

make_instr_func(call_near_indirect)
{
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, 4, cpu.eip);
	int len = 1;
	opr_src.data_size = data_size;
	len += modrm_rm(eip + 1, &opr_src);
	operand_read(&opr_src);
	cpu.eip = opr_src.val;
	print_asm_1("call", "", len, &opr_src);
	return 0;
}

make_instr_func(call_far)
{
	opr_src.type = OPR_IMM;
	opr_src.sreg = SREG_CS;
	opr_src.data_size = 16;
	opr_src.addr = eip + 1 + data_size / 8;
	opr_dest.type = OPR_IMM;
	opr_dest.sreg = SREG_CS;
	opr_dest.data_size = data_size;
	opr_dest.addr = eip + 1;
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, data_size / 8, opr_src.val);
	cpu.esp -= 4;
	vaddr_write(cpu.esp, SREG_SS, 4, cpu.eip);
	operand_read(&opr_src);
	operand_read(&opr_dest);
	cpu.eip += opr_dest.val;
	print_asm_2("call", "", 1 + data_size / 8 + 2, &opr_dest, &opr_src);
	return 0;
}

make_instr_func(call_far_indirect)
{
	opr_src.type = OPR_MEM;
	opr_src.sreg = SREG_CS;
	opr_src.data_size = 16;
	opr_src.addr = eip + 1 + data_size / 8;
	opr_dest.type = OPR_IMM;
	opr_dest.sreg = SREG_CS;
	opr_dest.data_size = data_size;
	opr_dest.addr = eip + 1;
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, data_size / 8, opr_src.val);
	cpu.esp -= 4;
	vaddr_write(cpu.esp, SREG_SS, 4, cpu.eip);
	operand_read(&opr_src);
	operand_read(&opr_dest);
	cpu.eip = opr_dest.val;
	print_asm_2("call", "", 1 + data_size / 8 + 2, &opr_dest, &opr_src);
	return 0;
}