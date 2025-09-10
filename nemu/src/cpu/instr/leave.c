#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
make_instr_func(leave)
{
    int len = 1;
    OPERAND ebp, esp;
    ebp.data_size = esp.data_size = data_size;
    ebp.type = esp.type = OPR_REG;
    ebp.addr = REG_EBP;
    esp.addr = REG_ESP;
    operand_read(&ebp);
    esp.val = ebp.val;
    operand_write(&esp);
    if (data_size == 16)
        ebp.val = vaddr_read(esp.addr, esp.val, 2);
    else
        ebp.val = vaddr_read(esp.addr, esp.val, 4);
    esp.val += data_size / 8;
    operand_write(&esp);
    operand_write(&ebp);
    print_asm_0("leave", "", len);
    return len;
}