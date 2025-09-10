#include "cpu/instr.h"
/*
Put the implementations of `lea' instructions here.
*/
make_instr_func(lea_rm2r_v)
{
    int len = 1;
    OPERAND r, rm;
    len += modrm_r_rm(eip + 1, &r, &rm);
    r.data_size = data_size;
    rm.data_size = data_size;
    operand_read(&rm);
    r.val = rm.addr;
    operand_write(&r);
    print_asm_2("lea", "", len, &rm, &r);
    return len;
}