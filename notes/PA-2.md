### PA-2 程序的执行：笔记

kouylty



写在最前面：这个是本人自己写 PA 时写的笔记，有提到一些知识点，内容也比较自由。对于实验手册中要求的内容，其中都有涵盖，会在段首写上 “**要求的内容**” 标志一下。



#### PA-2.1 指令解码与执行

在这一阶段，我们想让一个二进制可执行文件在 $\tt{NEMU}$ 上运行，我们要做的就是根据指令集进行解码（decode）、翻译成汇编语言然后执行。

**要求的内容：** 指令（instruction）在计算机中就是一串二进制数，跟数据一样存在内存中。

一条指令大体由两个部分组成：操作码（opcode）和操作数（operand）。想要对一条指令进行解码和翻译，大致要做几个过程：翻译操作码，提取操作数，进行操作（包括运算和修改数值等）。

在 $\tt{NEMU}$ 中，对于不同的操作数类型，我们会分配不同的函数名称便于解码，例如 `i` 表示立即数、`r` 表示寄存器、`rm` 表示寄存器或内存、`a` 表示特殊寄存器 `eax` 等等。同时，对于不同的操作数长度，我们也会给指令加上后缀来区分，例如 `b` 代表 $8$ 位、`v` 代表 $16$ 位或 $32$ 位等等。

对于操作码的翻译，我们建了一个数组 `opcode_entry[]`  存储函数指针（`opcode.c`）。对于大多数指令，操作码占一个字节，直接找到对应位置填上就好。有少部分指令的操作码有两个字节，这部分指令操作码的第一字节一定是 `0f`，因此我们在 `0x0f` 中填的是一个叫 `opcode_2_byte` 的函数组指针，指向第二字节的函数。有一些操作码对应不止一个指令，我们就会把这些指令打包成组，这些指令的第二个字节都会有编号告诉我们是本组的第几条，我们对应顺序填上就好。当然，如果仅仅按照 $\tt{80386}$ 中 instruction details 一个一个填的话是不够的。比如说对 push 指令，`push_r_v` 在细则中仅对应 `50`（第 $367$ 页），但实际上它应该对应 `50` 到 `57`，也就是说同一个函数可以对应连续的一段操作码。怎么发现的？在手册中有一个附录 A：Opcode Map，里面讲了每一个操作码对应的指令，在那里可以看到完整的内容。

翻译完操作码后我们就要提取操作数了。在 `modrm.c` 中提供了若干不同类型的提取函数，例如 `modrm_r_rm` 就是提取两个操作数，类型分别是 `r` 和 `rm`，其余以此类推。当然，在 `instr_helper.h` 中，提供了几个 `decode_operand` 的宏，对于大部分指令可以直接使用。在这部分还提供了 `make_instr_impl` 等宏，这些是为了简化后续编写指令函数而准备的。

接下来是主体部分，要编写每个指令函数的内容。第一个是一系列 alu 指令。以 `add` 为例，手册里要求支持`imm` 和 `rm`，`imm` 和 `a`，`r` 和 `rm` 相加。具体内容就是读出两个操作数，符号扩展到等长，加，最后再写回。

```c
#include "cpu/instr.h"

static void instr_execute_2op() 
{
	operand_read(&opr_src);
	operand_read(&opr_dest);
	if(opr_src.data_size != opr_dest.data_size)
		opr_src.val = sign_ext(opr_src.val, opr_src.data_size);
	opr_dest.val = alu_add(opr_src.val, opr_dest.val, opr_dest.data_size);
	operand_write(&opr_dest);
}

make_instr_impl_2op(add, r, rm, b)
make_instr_impl_2op(add, r, rm, v)
make_instr_impl_2op(add, rm, r, b)
make_instr_impl_2op(add, rm, r, v)
make_instr_impl_2op(add, i, rm, b)
make_instr_impl_2op(add, i, rm, v)
make_instr_impl_2op(add, i, rm, bv)
make_instr_impl_2op(add, i, r, b)
make_instr_impl_2op(add, i, a, b)
make_instr_impl_2op(add, i, a, v)
```

其他 alu 指令，包括 `adc`、`sub`、`sbb`、`shl`、`sar`、`shr`、`and`、`or`、`xor`，都是一样的写法。而且对于 `cmp` 和 `test` 两个比较指令来说也是这样，只不过这两个最后不用写回罢了。

**要求的内容：** 如果去掉 `instr_execute` 前的 `static` 关键字会怎么样？会在链接时报错。函数名是一个 strong symbol，如果不加 `static`，那么同一个名字在最后的可执行文件中就只会有一份（存在 `.symtab` 中）。对这么多同名量，如果是一个 strong 和若干 weak，那就只选择 strong 覆盖掉 weak；如果全是 weak 就随机选一个覆盖掉其他的；如果有多个 strong，就报错。而如果把 `static` 加上，这些函数名就会存到 `.bss` 中，每个函数标志上所属文件然后存一份，这样不同文件（不同指令）的同名函数就会都保留下来，然后按照需求分开调用。

第二个是 `mov` 指令，大部分指令也是类似的直接使用一套宏就好。对于有一些指令，要进行立即数的零扩展或符号扩展，要单独实现一下。注意最后返回的是指令长度，在 `exec_inst` 中，让 PC（`eip`）加上也就是下一条指令的地址。

```c
make_instr_func(mov_zrm162r_l) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);
        operand_read(&rm);
        r.val = rm.val;
        operand_write(&r);
		print_asm_2("mov", "", len, &rm, &r);
        return len;
}
```

第三个是直接跟程序栈有关的指令，包括要压入栈的 `push` 和要弹出栈的 `pop`、`leave` 等。程序栈本质上是内存中一段连续的空间，寄存器 `esp` 维护栈顶的地址，小地址在栈顶，大地址在栈底。每次压入或弹出栈都要对应修改 `esp`。对 `push` 而言，要做的就是读出操作数，把值写入栈顶，修改 `esp`。对 `pop` 而言，要做的就是读出栈顶的值，弹栈，然后写入操作数中。`leave` 也类似，根据手册实现即可，这条指令不是很常用。当然，`push` 和 `pop` 还有压入或弹出至段寄存器的操作，目前还未实现段寄存器相关内容，故先跳过。

```c
#include "cpu/instr.h"

static void instr_execute_1op()
{
    operand_read(&opr_src);
    cpu.esp -= data_size / 8;
    vaddr_write(cpu.esp, SREG_SS, data_size / 8, opr_src.val);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, rm, v)
```

```c
#include "cpu/instr.h"

static void instr_execute_1op()
{
    opr_src.val = vaddr_read(cpu.esp, SREG_SS, data_size / 8);
    cpu.esp += data_size / 8;
    operand_write(&opr_src);
}

make_instr_impl_1op(pop, rm, v)
make_instr_impl_1op(pop, r, v)
```

第四个是跟跳转相关的指令，包括 `jmp`、`call`、`ret` 等。跳转分为直接跳转和间接跳转，直接跳转的目标地址直接通过立即数写在指令里，但这个目标地址是相对地址，还要加上指令完成后的 PC （内存中下一条指令的起始地址）才是真正要跳转的绝对地址，因此指令函数正常返回指令长度就行。间接跳转的目标地址会存在寄存器或内存中，要先读出来。这个地址就直接是绝对目标地址了，因此为了避免偏差，这些函数要返回 $0$。这里面还有一些细节，如果操作数是 $16$ 位，属于 `near`，要先符号扩展到对应位数（对 $\tt{NEMU}$ 来说是 $32$ 位）；如果操作数是 $8$ 位，属于 `short`，不用扩展。还有一类是 `far`，涉及到段寄存器，也先暂且不涉及。`jmp` 的内容就以上这么多，对于 `call` 而言，还要把 PC 压入栈中，对应于 `ret` 的返回。这里要注意压入的 PC 是当前指令完成后的，即内存中下一条指令的起始地址，这样 `ret` 后才能正常向下，否则就又回到 `call` 的起始位置，会死循环。`ret` 类似，因为从栈中取出的就直接是下一条指令的起始地址，因此要返回 $0$。

```c
#include "cpu/instr.h"

make_instr_func(call_near)
{
	OPERAND rel;
	rel.type = OPR_IMM;
	rel.sreg = SREG_CS;
	rel.data_size = data_size;
	rel.addr = eip + 1;
	operand_read(&rel);
	int offset = sign_ext(rel.val, data_size);
	print_asm_1("call", "", 1 + data_size / 8, &rel);
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, data_size / 8, cpu.eip+1+data_size/8);
	cpu.eip += offset;
	return 1 + data_size / 8;
}

make_instr_func(call_near_indirect)
{
	int len = 1;
	opr_src.data_size = data_size;
	len += modrm_rm(eip + 1, &opr_src);
	operand_read(&opr_src);
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, 4, cpu.eip+len);
	cpu.eip = opr_src.val;
	print_asm_1("call", "", len, &opr_src);
	return 0;
}

make_instr_func(call_far)
{
	opr_src.type = OPR_IMM;
	opr_src.sreg = SREG_CS;
	opr_src.data_size = 16;
	opr_src.addr = eip + 1 + data_size / 8;
	opr_dest.type = OPR_IMM;
	opr_dest.sreg = SREG_CS;
	opr_dest.data_size = data_size;
	opr_dest.addr = eip + 1;
	operand_read(&opr_src);
	operand_read(&opr_dest);
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, data_size / 8, opr_src.val);
	cpu.esp -= 4;
	vaddr_write(cpu.esp, SREG_SS, 4, cpu.eip+1+data_size/8+2);
	cpu.eip += opr_dest.val;
	print_asm_2("call", "", 1 + data_size / 8 + 2, &opr_dest, &opr_src);
	return 0;
}

make_instr_func(call_far_indirect)
{
	opr_src.type = OPR_MEM;
	opr_src.sreg = SREG_CS;
	opr_src.data_size = 16;
	opr_src.addr = eip + 1 + data_size / 8;
	opr_dest.type = OPR_IMM;
	opr_dest.sreg = SREG_CS;
	opr_dest.data_size = data_size;
	opr_dest.addr = eip + 1;
	operand_read(&opr_src);
	operand_read(&opr_dest);
	cpu.esp -= data_size / 8;
	vaddr_write(cpu.esp, SREG_SS, data_size / 8, opr_src.val);
	cpu.esp -= 4;
	vaddr_write(cpu.esp, SREG_SS, 4, cpu.eip+1+data_size/8+2);
	cpu.eip = opr_dest.val;
	print_asm_2("call", "", 1 + data_size / 8 + 2, &opr_dest, &opr_src);
	return 0;
}
```

By the way，稍微记录下我的调试经历。在工作过程中，除了会遇到 invalid opcode 的错误，还会遇到段错误（segmentation fault），通过 gdb 打断点，发现都是出现在 `vaddr_read` 中，信号都是 `SIGSEGV`，说明都是内存越界。这就提醒我们检查有内存写入的部分，尤其是写入栈的部分。几个写过的 bug 包括： `push` 前没有读出操作数、绝对跳转时函数返回值还是 `len` 而不是 $0$、`call` 时压入栈的是 `call` 指令的地址。这些都是通过 gdb 查出来的。让我们说：“谢谢 gdb！”

<img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\debug2(1).png" style="zoom:50%;" />

第五个是跟条件码（condition code）相关的指令，包括 `setcc` 和 `jmpcc`。这边的主要工作是设计好条件码，例如小于等于（le）的条件码为 `ZF || SF!=OF` 等等。这些根据手册中一条一条设置即可。

还有一些细节的指令，像 `xchg` 等等，这边就不赘述，看手册类似的实现即可。

Testing...

<img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\test2(1).png" style="zoom:30%;" /> <img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\test2(2).png" style="zoom:30%;" />

<img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\test2(3).png" style="zoom:30%;" /> <img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\test2(4).png" style="zoom:30%;" />

**要求的内容：** 为什么 `test-float` 会 hit BAD trap？我们查看 `NEMU_trap` 会发现，当寄存器 `eax` 等于 $0$ 时，会输出 GOOD，否则输出 BAD。这就说明 `test-float` 程序运行的返回值不是 $0$，也就是说 main 函数并不是以 `return 0` 或 `exit(0)` 结尾。如果这样做可能会导致程序的异常行为（最经典的，在 NOI 中不 `return 0` 可能会爆零 qaq）。这就提醒我们，main 函数最后一定要 `return 0`。

至此，PA-2.1 完成。