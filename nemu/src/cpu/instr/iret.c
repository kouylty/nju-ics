#include "cpu/instr.h"
/*
Put the implementations of `iret' instructions here.
*/
make_instr_func(iret)
{
#ifdef IA32_INTR
    cpu.eip = vaddr_read(cpu.esp,SREG_SS,4);
    cpu.esp += 4;
    cpu.cs.val = vaddr_read(cpu.esp,SREG_SS,2);
    cpu.esp += 4;
    cpu.eflags.val = vaddr_read(cpu.esp,SREG_SS,4);
    cpu.esp += 4;
#endif
    print_asm_0("iret","",1);
    return 0;
}