#ifndef __INSTRUCTIONS_H__
#define __INSTRUCTIONS_H__

#include "nemu.h"
#include "cpu/cpu.h"
#include "cpu/fpu.h"
#include "cpu/modrm.h"
#include "cpu/operand.h"
#include "cpu/instr_helper.h"
#include "memory/memory.h"
#include <stdio.h>
#include <cpu-ref/instr_ref.h>


extern uint8_t data_size;

#include "cpu/instr/mov.h"
#include "cpu/instr/jmp.h"
#include "cpu/instr/shift.h"
#include "cpu/instr/flags.h"
#include "cpu/instr/group.h"
#include "cpu/instr/special.h"
#include "cpu/instr/x87.h"

/* TODO: add more instructions here */

#include "cpu/instr/adc.h"  // done
#include "cpu/instr/add.h"  // done
#include "cpu/instr/and.h"  // done
#include "cpu/instr/call.h"	// done
#include "cpu/instr/cli.h"  // done
#include "cpu/instr/cmp.h"  // done
#include "cpu/instr/cmps.h"	// done
#include "cpu/instr/dec.h"  // done
#include "cpu/instr/div.h"  // done
#include "cpu/instr/flags.h"	//done
#include "cpu/instr/group.h"	// done
#include "cpu/instr/in.h"
#include "cpu/instr/inc.h"  // done
#include "cpu/instr/int_.h"
#include "cpu/instr/iret.h"
#include "cpu/instr/jcc.h"	// done
#include "cpu/instr/jmp.h"  // done
#include "cpu/instr/lea.h"  // done
#include "cpu/instr/leave.h"    // done
#include "cpu/instr/lgdt.h"
#include "cpu/instr/lidt.h"
#include "cpu/instr/mov.h"  // done
#include "cpu/instr/mul.h"  // done
#include "cpu/instr/neg.h"  // done
#include "cpu/instr/not.h"  // done
#include "cpu/instr/or.h"   // done
#include "cpu/instr/out.h"
#include "cpu/instr/pop.h"  // done
#include "cpu/instr/push.h" // done
#include "cpu/instr/ret.h"	// done
#include "cpu/instr/sbb.h"  // done
#include "cpu/instr/setcc.h"	// done
#include "cpu/instr/shift.h"    // done
#include "cpu/instr/special.h"	// done
#include "cpu/instr/sti.h"
#include "cpu/instr/stos.h"	// done
#include "cpu/instr/sub.h"  // done
#include "cpu/instr/test.h" // done
#include "cpu/instr/xchg.h"	// done
#include "cpu/instr/xor.h"  // done

#endif
