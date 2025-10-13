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

make_instr_func(popa)
{
    uint32_t reg_val[8];
    for(int i=7;i>=0;i--)
    {
        reg_val[i] = vaddr_read(cpu.esp, SREG_SS, 4);
        cpu.esp += 4;
    }
    cpu.eax = reg_val[0];
    cpu.ecx = reg_val[1];
    cpu.edx = reg_val[2];
    cpu.ebx = reg_val[3];
    // cpu.esp = reg_val[4];
    cpu.ebp = reg_val[5];
    cpu.esi = reg_val[6];
    cpu.edi = reg_val[7];
    print_asm_0("popa", "", 1);
    return 1;
}