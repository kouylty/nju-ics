#ifndef __INSTR_JMP_H__
#define __INSTR_JMP_H__

make_instr_func(jmp_near);
make_instr_func(jmp_near_indirect);
make_instr_func(jmp_short);
#ifdef IA32_SEG
make_instr_func(jmp_far);
#endif

#endif
