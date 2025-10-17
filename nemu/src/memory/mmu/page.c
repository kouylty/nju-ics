#include "cpu/cpu.h"
#include "memory/memory.h"

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
#ifdef IA32_PAGE
	uint32_t dir = laddr >> 22 & 0x3ff;
	uint32_t page = laddr >> 12 & 0x3ff;
	uint32_t offset = laddr & 0xfff;
	PDE *pde = (PDE *)(hw_mem + (cpu.cr3.dir << 12) + dir * 4);
	assert(pde->present == 1);
	PTE *pte = (PTE *)(hw_mem + (pde->page_frame << 12) + page *4);
	assert(pte->present == 1);
	return (pte->page_frame << 12) | offset;
#else
	return laddr;
#endif
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}