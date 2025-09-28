#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	uint32_t base = cpu.segReg[sreg].base;
	return offset + base;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
	SegDesc desc;
	desc.val[0] = laddr_read(cpu.gdtr.base + cpu.segReg[sreg].index * 8, 4);
	desc.val[1] = laddr_read(cpu.gdtr.base + cpu.segReg[sreg].index * 8 + 4, 4);
	cpu.segReg[sreg].base = (desc.base_31_24 << 24) | (desc.base_23_16 << 16) | desc.base_15_0;
	cpu.segReg[sreg].limit = (desc.limit_19_16 << 16) | desc.limit_15_0;
	cpu.segReg[sreg].type = desc.type;
	cpu.segReg[sreg].privilege_level = desc.privilege_level;
	cpu.segReg[sreg].soft_use = desc.soft_use;
	assert(cpu.segReg[sreg].base == 0);
	assert(cpu.segReg[sreg].limit == 0xfffff);
	assert(desc.granularity == 1);
}
