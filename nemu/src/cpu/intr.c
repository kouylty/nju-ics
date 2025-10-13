#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
	OPERAND r;
	r.type = OPR_MEM;
	r.sreg = SREG_SS;
	r.data_size = 32;
	cpu.esp -= 4;
	r.addr = cpu.esp;
	r.val = cpu.eflags.val;
	operand_write(&r);
	cpu.esp -= 4;
	r.addr = cpu.esp;
	r.val = cpu.cs.val;
	operand_write(&r);
	cpu.esp -= 4;
	r.addr = cpu.esp;
	r.val = cpu.eip;
	operand_write(&r);
	assert(cpu.cr0.pe==1);
	GateDesc* gds = (GateDesc*)(hw_mem + page_translate(segment_translate(cpu.idtr.base + 8*intr_no, SREG_DS)));
    if(gds->type==0xe)
        cpu.eflags.IF = 0;
    cpu.eip = ((gds->offset_31_16)<<16) + gds->offset_15_0;
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
