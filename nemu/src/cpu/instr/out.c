#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `out' instructions here.
*/
make_instr_func(out_b)
{
#ifdef IA32_INTR
    OPERAND r, port;
    r.data_size = 8;
    r.type = OPR_REG;
    r.sreg = SREG_CS;
    r.addr = REG_AL;
    port.data_size = 16;
    port.type = OPR_REG;
    port.addr = REG_DX;
    operand_read(&r);
    operand_read(&port);
    pio_write(port.val, 1, r.val);
#endif
    print_asm_0("out", "", 2);
    return 1;
}

make_instr_func(out_v)
{
#ifdef IA32_INTR
    OPERAND r, port;
    r.data_size = data_size;
    r.type = OPR_REG;
    r.sreg = SREG_CS;
    r.addr = REG_AL;
    port.data_size = 16;
    port.type = OPR_REG;
    port.addr = REG_DX;
    operand_read(&r);
    operand_read(&port);
    pio_write(port.val, data_size / 8, r.val);
#endif
    print_asm_0("out", "", 1 + data_size / 8);
    return 1;
}