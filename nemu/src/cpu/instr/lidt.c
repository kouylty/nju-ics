#include "cpu/instr.h"
/*
Put the implementations of `lidt' instructions here.
*/
make_instr_func(lidt)
{
    int len = 1;
#ifdef IA32_INTR
    OPERAND m;
    m.data_size = 16;
    len += modrm_rm(eip + 1, &m);
    operand_read(&m);
    cpu.idtr.limit = m.val;
    m.data_size = 32;
    m.addr += 2;
    operand_read(&m);
    cpu.idtr.base = m.val;
    print_asm_1("lidt", "", len, &m);
#endif
    return len;
}