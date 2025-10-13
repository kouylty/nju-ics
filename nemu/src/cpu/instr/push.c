#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op()
{
    operand_read(&opr_src);
    cpu.esp -= data_size / 8;
    vaddr_write(cpu.esp, SREG_SS, data_size / 8, opr_src.val);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, rm, v)

make_instr_func(pusha)
{
    uint32_t temp = cpu.esp;
    uint32_t reg_val[8] = {cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, temp, cpu.ebp, cpu.esi, cpu.edi};
    for(int i=0;i<8;i++)
    {
        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, reg_val[i]);
    }
    print_asm_0("pusha", "", 1);
    return 1;
}