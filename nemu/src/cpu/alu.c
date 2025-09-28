#include "cpu/cpu.h"

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = src + dest;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	cpu.eflags.CF = (s < dest);
	cpu.eflags.OF = ((dest>>(data_size-1))==0 && (src>>(data_size-1))==0 && (s>>(data_size-1))==1) 
				|| ((dest>>(data_size-1))==1 && (src>>(data_size-1))==1 && (s>>(data_size-1))==0);
	return s;
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = src + dest + cpu.eflags.CF;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	cpu.eflags.CF = (s < dest) || (cpu.eflags.CF && s == dest);
	cpu.eflags.OF = ((dest>>(data_size-1))==0 && (src>>(data_size-1))==0 && (s>>(data_size-1))==1) 
				|| ((dest>>(data_size-1))==1 && (src>>(data_size-1))==1 && (s>>(data_size-1))==0);
	return s;
#endif
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = dest - src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	cpu.eflags.CF = (dest < src);
	cpu.eflags.OF = ((dest>>(data_size-1))==1 && (src>>(data_size-1))==0 && (s>>(data_size-1))==0) 
				|| ((dest>>(data_size-1))==0 && (src>>(data_size-1))==1 && (s>>(data_size-1))==1);
	return s;
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = dest - src - cpu.eflags.CF;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	cpu.eflags.CF = (dest < src) || (cpu.eflags.CF && dest == src);
	cpu.eflags.OF = ((dest>>(data_size-1))==1 && (src>>(data_size-1))==0 && (s>>(data_size-1))==0) 
				|| ((dest>>(data_size-1))==0 && (src>>(data_size-1))==1 && (s>>(data_size-1))==1);
	return s;
#endif
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint64_t s = (uint64_t)src * (uint64_t)dest;
	s &= (0xffffffffffffffff >> (64-data_size-data_size));
	uint64_t mx = 0xffffffff >> (32-data_size);
	cpu.eflags.OF = (s > mx);
	cpu.eflags.CF = (s > mx);
	return s;
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	int64_t s = (int64_t)src * (int64_t)dest;
	int64_t mx = (1LL << data_size) - 1, mn = -(1LL << data_size);
	cpu.eflags.OF = (s > mx || s < mn);
	cpu.eflags.CF = (s > mx || s < mn);
	return s;
#endif
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	assert(src!=0);
	uint32_t s = dest / src;
	s &= 0xffffffff >> (32-data_size);
	return s;
#endif
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	assert(src!=0);
	int32_t s = dest / src;
	return s;
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	uint32_t s = dest % src;
	return s;
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	int32_t s = dest % src;
	return s;
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint32_t s = dest & src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = 0;
	cpu.eflags.OF = 0;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint32_t s = dest ^ src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = 0;
	cpu.eflags.OF = 0;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint32_t s = dest | src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = 0;
	cpu.eflags.OF = 0;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
#endif
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint32_t s = dest << src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = (dest >> (data_size - src)) & 0x1;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
#endif
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint32_t s = dest >> src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = (dest >> (src - 1)) & 0x1;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
#endif
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint8_t sign = (dest >> (data_size - 1)) & 0x1;
	uint32_t s = dest >> src;
	if(sign) s |= (0xffffffff << (data_size - src));
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = (dest >> (src - 1)) & 0x1;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
/*
	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
	fflush(stdout);
	assert(0);
	return 0;
*/
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = dest << src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = (dest >> (data_size - src)) & 0x1;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
#endif
}
