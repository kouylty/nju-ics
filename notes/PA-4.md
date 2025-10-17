### PA-4 异常、中断与IO：笔记

kouylty



写在最前面：这个是本人自己写 PA 时写的笔记，有提到一些知识点，内容也比较自由。对于实验手册中要求的内容，其中都有涵盖，会在段首写上 “**要求的内容**” 标志一下。



#### PA-4.1 异常的中断与相应

一个完整的计算机系统除了要能执行用户程序，还必须要能对系统状态的变化做出反应，这些系统状态不是被内部程序变量捕获的，也不一定要和程序的执行相关。为了对这些情况作出反应，现代系统通过使控制流发生突变来实现。而这些突变就是异常控制流（exceptional control flow, ecf）。顾名思义，异常控制流与异常（exception）有关。一般而言，异常可以分为四类：中断（interrupt）、陷阱（trap）、故障（fault）、终止（abort）。中断通常是来自 IO 设备的信号，并且总是返回到下一条指令。陷阱是有意的异常，通常与系统调用有关，例如请求内核等，处理器还专门有一条指令 `syscall n`，来控制系统调用。故障是潜在的可恢复错误，例如缺页（page fault）等。终止就是不可恢复的错误，处理程序会将控制返回给一个 `abort` 例程并终止应用程序。每一个异常都会有一个异常号，来标识异常的类型和内容。在 $\tt{NEMU}$ 中，我们只考虑中断和陷阱。

遇到异常时，会打断原来程序的执行，转而执行操作系统提供的针对这些事件的处理程序，执行完之后再视情况返回。这些异常处理程序是预先写进内核里的，遇到异常时直接调用。想要调用这些程序，首先就要知道这些程序的地址，这都是通过中断描述符表（interrupt descriptor table, IDT）实现的。IDT 的每一个表项存储一个异常处理程序的虚拟地址，要使用时，就用异常号访存，让 `PC` 跳转到对应的处理程序。

系统调用直观上就相当于一个特殊的函数调用，所以系统运行时栈也要像正常函数调用一样存储调用者（caller）的信息，因此转移到异常处理程序也有一套管理栈的指令，包括 `pusha`、`popa` 等，当然还有调用指令 `int` 和返回指令 `iret`。只不过要比程序内函数调用存储更多的信息，我们要把原来用户程序的所有运行上下文（context）都存下来，这样才能在处理完异常后丝滑地回到应用程序继续执行。运行上下文包括所有的寄存器、段寄存器、标志位等等。对于 `int` 指令，要将段寄存器和标志位压入栈中，再进行查表和跳转。而 `pusha` 指令将所有寄存器值按顺序压入栈中。

```c
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
```

```c
make_instr_func(pusha)
{
    uint32_t temp = cpu.esp;
    uint32_t reg_val[8] = {cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, temp, cpu.ebp, cpu.esi, cpu.edi};
    for(int i=0;i<8;i++)
    {
        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_SS, 4, reg_val[i]);
    }
    print_asm_0("pusha", "", 1);
    return 1;
}
```

`iret` 和 `popa` 指令就是把上述过程反过来做一遍弹栈，不多赘述。

**要求的内容：** 总的来说，从指令 `int $0x80` 开始，我们就从正常用户程序跳开，开始执行预设好的异常处理程序。打印出所有的指令，开始分析。`int` 指令（`opcode` 为 `0xcd`）将部分运行上下文压入栈，包括 `eflags`、`cs`、`eip` 等，然后再通过中断描述符表将 `PC` 移到异常处理程序的起始位置。从下图可以看到，在 `int` 指令后，先进行了 `push` 和 `pusha` （`opcode`为 `0x60`）的压栈指令，目的是将所有寄存器值先压入栈，以便记录运行上下文的信息（其实直观上调用异常处理程序就相当于一个特殊的函数调用，只不过是系统调用）。然后正常执行异常处理程序的内容，我们发现就是在这当中触发了一次 `nemu_trap`，并使其输出 `Hello, world!`。在异常处理程序结束后，我们发现程序执行了 `popa`（`opcode` 为 `0x61`）的弹栈指令，清空运行上下文并通过 `iret` 指令（`opcode` 为 `0xcf`）返回调用者，也就是原来的用户程序。

<img src="https://raw.githubusercontent.com/kouylty/nju-ics/master/notes/test4(2).png" style="zoom:50%;" alt="【若图片无法加载，详见https://github.com/kouylty/nju-ics/blob/master/notes/test4(2).png】">  <img src="https://raw.githubusercontent.com/kouylty/nju-ics/master/notes/test4(3).png" style="zoom:50%;" alt="【若图片无法加载，详见https://github.com/kouylty/nju-ics/blob/master/notes/test4(3).png】">

顺带一提，如果在测试时出现了下面的结果，通常是因为 `pusha` 或 `popa` 实现有误。 在这里只能显式地读写内存，如果直接用写好的 `push` 和 `pop` 指令函数会有问题。

<img src="https://raw.githubusercontent.com/kouylty/nju-ics/master/notes/test4(1).png" style="zoom:50%;" alt="【若图片无法加载，详见https://github.com/kouylty/nju-ics/blob/master/notes/test4(1).png】">

至此，对于陷阱，或者说是系统调用，我们就处理好了。接下来我们来看外接设备引发的中断。为了及时处理中断，我们在每一条指令结束后都会检查有没有新的中断，有的话就先处理。

```c
void do_intr()
{
	if (cpu.intr && cpu.eflags.IF)
	{
		is_nemu_hlt = false;
		uint8_t intr_no = i8259_query_intr_no();
		assert(intr_no != I8259_NO_INTR);
		i8259_ack_intr();
		raise_intr(intr_no);
	}
}
```

测试一下，发现有一个 `panic`，我们来看看触发它的机制是什么样的。

<img src="https://raw.githubusercontent.com/kouylty/nju-ics/master/notes/test4(4).png" style="zoom:50%;" alt="【若图片无法加载，详见https://github.com/kouylty/nju-ics/blob/master/notes/test4(4).png】">

这个 `panic` 在函数 `irq_handle` 中，并且对应的 `irq_id` 等于 $1000$，这个值是 $\tt{NEMU}$ 专门为时钟中断设置的异常号。在编译过程中，通过打桩（interpositioning）将 `do_irq.S` 嵌入到内核中。

```shell
gcc -m32 -MMD -I./include -I../include -c -o src/irq/do_irq.o src/irq/do_irq.S
```

```assembly
.globl vec0;    vec0:   pushl $0;  pushl    $0; jmp asm_do_irq
.globl vec1;    vec1:   pushl $0;  pushl    $1; jmp asm_do_irq
.globl vec2;    vec2:   pushl $0;  pushl    $2; jmp asm_do_irq
.globl vec3;    vec3:   pushl $0;  pushl    $3; jmp asm_do_irq
.globl vec4;    vec4:   pushl $0;  pushl    $4; jmp asm_do_irq
.globl vec5;    vec5:   pushl $0;  pushl    $5; jmp asm_do_irq
.globl vec6;    vec6:   pushl $0;  pushl    $6; jmp asm_do_irq
.globl vec7;    vec7:   pushl $0;  pushl    $7; jmp asm_do_irq
.globl vec8;    vec8:              pushl    $8; jmp asm_do_irq
.globl vec9;    vec9:   pushl $0;  pushl    $9; jmp asm_do_irq
.globl vec10;   vec10:             pushl   $10; jmp asm_do_irq
.globl vec11;   vec11:             pushl   $11; jmp asm_do_irq
.globl vec12;   vec12:             pushl   $12; jmp asm_do_irq
.globl vec13;   vec13:             pushl   $13; jmp asm_do_irq
.globl vec14;   vec14:             pushl   $14; jmp asm_do_irq

.globl vecsys; vecsys:  pushl $0;  pushl $0x80; jmp asm_do_irq

.globl irq0;     irq0:  pushl $0;  pushl $1000; jmp asm_do_irq
.globl irq1;     irq1:  pushl $0;  pushl $1001; jmp asm_do_irq
.globl irq2;     irq2:  pushl $0;  pushl $1002; jmp asm_do_irq
.globl irq14;   irq14:  pushl $0;  pushl $1014; jmp asm_do_irq
.globl irq_empty;
			irq_empty:	pushl $0;  pushl   $-1; jmp asm_do_irq

.globl asm_do_irq
.extern irq_handle

asm_do_irq:
	pushal
	pushl %esp
	call irq_handle
	addl $4, %esp
	popal
	addl $8, %esp
	iret
```

研究一下 `do_irq.S` 中的内容。最上方的表格就是我们的 IDT，可以看到异常号为 $0-14$ 都是故障，$80$ 是陷阱，$1000,1001,1002$ 和 $1014$ 是中断。这其中的核心是 `asm_do_irq` 函数。其中的 `irq_handle` 就是处理异常的核心程序，`pushal` 和 `popal` 就是记录原程序的上下文并返回。再 `raise_intr` 访问 IDT 后，`PC` 就会自动指向对应表项，先将异常号压入栈，再执行 `asm_do_irq`。

**要求的内容：** 在 `call irq_handle` 前，栈中从底到顶依次为 `eax`，`ecx`，`edx`，`ebx`，`esp`，`ebp`，`esi`，`edi` 和 `esp`。 `esp` 指向栈顶。为什么会出现一句 `pushl %esp` 呢？这条指令的语义是把执行前的 `esp` 寄存器值压入栈，让它作为 `irq_handle` 的参数使用（这是必要的，因为 `routine` 指向的函数会用到栈帧），同时以便程序能找到刚刚 `pushal` 保存的寄存器上下文（`pt_regs` 样式的结构）。

这样，我们就理清了整个控制流的流程，把 `panic` 删去就通过测试。

**要求的内容：** 最后，我们来比较一下 $\tt{NEMU}$ 及其内核在相应系统调用和时钟中断的异同。两者的相同点是执行的都是系统预定义好的异常处理程序等等。两者的第一类不同点在于异常出现的位置，系统调用的异常是直接在系统内部，而类似于时钟中断的异常是来自外部接口的异常，两者的第二个不同点在于获取异常处理程序的方式，系统调用是直接通过 `int` 指令查找中断描述表来找到异常处理程序的起始位置，索引是通过 `idtr` 和 `intr_no`，而时钟中断是 CPU 执行指令的过程中异步检查并处理，不设置单独的指令。

至此，PA-4.1 完成。