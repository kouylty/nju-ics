#include "cpu/instr.h"
/*
Put the implementations of `lgdt' instructions here.
*/
make_instr_func(lgdt)
{
    int len = 1;
    OPERAND m;
    m.data_size = 16;
    len += modrm_rm(eip + 1, &m);
#ifdef IA32_SEG
    operand_read(&m);
    cpu.gdtr.limit = m.val;
    m.data_size = 32;
    m.addr += 2;
    operand_read(&m);
    cpu.gdtr.base = m.val;
#endif
    print_asm_1("lgdt", "", len, &m);
    return len;
}