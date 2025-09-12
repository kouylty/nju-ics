#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(ret_near)
{
	cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
	cpu.esp += 4;
	print_asm_0("ret", "", 1);
	return 0;
}

make_instr_func(ret_near_imm16)
{
	cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
	cpu.esp += 4;
	OPERAND imm;
	imm.type = OPR_IMM;
	imm.sreg = SREG_CS;
	imm.data_size = 16;
	imm.addr = eip + 1;
	operand_read(&imm);
	cpu.esp += imm.val;
	print_asm_1("ret", "", 1 + 2, &imm);
	return 0;
}

make_instr_func(ret_far)
{
	cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
	cpu.esp += 4;
#ifdef IA32_SEG
	cpu.cs.val = vaddr_read(cpu.esp, SREG_SS, 2);
	cpu.esp += 2;
	load_sreg(SREG_CS);
#endif
	print_asm_0("ret", "", 1);
	return 0;
}

make_instr_func(ret_far_imm16)
{
	cpu.eip = vaddr_read(cpu.esp, SREG_SS, 4);
	cpu.esp += 4;
#ifdef IA32_SEG
	cpu.cs.val = vaddr_read(cpu.esp, SREG_SS, 2);
	cpu.esp += 2;
	load_sreg(SREG_CS);
#endif
	OPERAND imm;
	imm.type = OPR_IMM;
	imm.sreg = SREG_CS;
	imm.data_size = 16;
	imm.addr = eip + 1;
	operand_read(&imm);
	cpu.esp += imm.val;
	print_asm_1("ret", "", 1 + 2, &imm);
	return 0;
}