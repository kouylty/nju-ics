#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `in' instructions here.
*/
make_instr_func(in_b)
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
    operand_read(&port);
    r.val = pio_read(port.val, 1);
    operand_write(&r);
#endif
    print_asm_0("in", "", 1);
    return 1;
}

make_instr_func(in_v)
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
    operand_read(&port);
    r.val = pio_read(port.val, data_size / 8);
    operand_write(&r);
#endif
    print_asm_0("in", "", 1);
    return 1;
}

make_instr_func(in_i2a_b)
{
#ifdef IA32_INTR
    OPERAND r, port;
    r.data_size = 8;
    r.type = OPR_REG;
    r.sreg = SREG_CS;
    r.addr = REG_AL;
    port.data_size = 8;
    port.type = OPR_IMM;
    operand_read(&port);
    r.val = pio_read(port.val, 1);
    operand_write(&r);
    print_asm_1("in", "b", 2 + port.data_size / 8, &port);
    return 1 + port.data_size / 8;
#else
    return 1;
#endif
}

make_instr_func(in_i2a_v)
{
#ifdef IA32_INTR
    OPERAND r, port;
    r.data_size = data_size;
    r.type = OPR_REG;
    r.sreg = SREG_CS;
    r.addr = REG_AL;
    port.data_size = 8;
    port.type = OPR_IMM;
    operand_read(&port);
    r.val = pio_read(port.val, data_size / 8);
    operand_write(&r);
    print_asm_1("in", "", 1 + port.data_size / 8, &port);
    return 1 + port.data_size / 8;
#else
    return 1;
#endif
}