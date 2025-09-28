### PA-3 存储管理：笔记

kouylty



写在最前面：这个是本人自己写 PA 时写的笔记，有提到一些知识点，内容也比较自由。对于实验手册中要求的内容，其中都有涵盖，会在段首写上 “**要求的内容**” 标志一下。



#### PA-3.1 Cache的模拟

CPU 访问主存是很缓慢的，不仅有读写的延迟，还会受到带宽（bandwidth）的限制。因此，在计算机中都会有一块离 CPU 更近的存储空间，它比较小，但 CPU 访问起来更快，这就是高速缓存（cache）。cache 的结构可以抽象为一个 $S\times B\times E$ 的数组，其中记 $S=2^s,B=2^b$，代表缓存一共有 $S$ 组，每一组中有 $E$ 行（cache line），每行有 $B$ 块（block）。对于一个指令，它的低 $b$ 位决定了这个地址的内容存储在哪个块中，更高的中间 $s$ 位指定这个地址的内容在缓存的哪一组，最高的剩余位（记作 `tag`）用来找到对应的缓存行。

一般情况下，计算机在访存时，会先去缓存里找，如果找不到再去主存里取，然后把主存里的值放到缓存里便于后续查找，当然这就提醒我们程序的局部性（locality）很重要。因为缓存的空间很小，所以经常要做替换。一般采用的替换方式是 LRU（least recently used），即替换掉最近最不常用的。所以对于一个缓存行，我们要记录下它的 `tag`，当前状态 `v`（valid），里面的值 `data`，还有最近一次使用的时间 `t`。

```c
typedef struct CacheLine {
	uint8_t v;
	int tag;
	int t;
	uint8_t *data;
} line_t;
typedef struct Cache{
	int S,B,E;
	line_t **line;
} cache_t;
```

在我们的 $\tt{NEMU}$ 中，要求建一个 $64KB$ 的缓存，有 $8$ 组，每行 block 的大小是 $64B$，这就意味着 $s=3,b=6,E=128$。

```c
void init_cache()
{
	cache.S = 1<<3;
	cache.B = 1<<6;
	cache.E = 128;
	cache.line = (line_t **)malloc(sizeof(line_t *) * cache.S);
	for(int i = 0; i < cache.S; i++) {
		cache.line[i] = (line_t *)malloc(sizeof(line_t) * cache.E);
		for(int j = 0; j < cache.E; j++) {
			cache.line[i][j].v = 0;
			cache.line[i][j].tag = -1;
			cache.line[i][j].t = 0;
			cache.line[i][j].data = (uint8_t *)malloc(sizeof(uint8_t) * cache.B);
			memset(cache.line[i][j].data, 0, sizeof(uint8_t) * cache.B);
		}
	}
}
```

接下来就是要实现缓存的读写。我们先找到目标地址的组数，用 `tag` 搜索对应行号，再在 block 组中读或写。如果找不到对应行号，就用 LRU 替换，从主存中取出一整段的值，然后更新每个行的 `t`。这里有一个小细节，因为每一块存的都是一个字节的值，但我们一般读写都是一个四字节的值。如果出现这四个字节出现在不同组的情况，就一个字节一个字节做 `cache_read` 或 `cache_write`。

```c
uint32_t cache_read(paddr_t paddr, size_t len)
{
	int tag = paddr / (cache.B * cache.S);
	int s = (paddr / cache.B) % cache.S;
	int b = paddr % cache.B;
	int u = -1;
	uint32_t ret = 0;
	if(b+len>cache.B) {
		for(int i=0;i<len;i++)
			ret |= ((uint32_t)cache_read(paddr+i, 1)) << (i*8);
		return ret;
	}
	for(int i=0;i<cache.E;i++) {
		if(cache.line[s][i].v && cache.line[s][i].tag == tag) {
			u = i;
			break;
		}
	}
	if(u == -1) {
		int mx = -1;
		for(int i=0;i<cache.E;i++) {
			if(!cache.line[s][i].v) {
				u = i;
				break;
			}
			if(cache.line[s][i].t > mx)
				mx = cache.line[s][i].t, u = i;
		}
		memcpy(cache.line[s][u].data, hw_mem + (paddr / cache.B) * cache.B, cache.B);
	}
	cache.line[s][u].v = 1;
	cache.line[s][u].t = 0;
	cache.line[s][u].tag = tag;
	for(int i=0;i<cache.E;i++)
		if(i != u && cache.line[s][i].v)
			cache.line[s][i].t++;
	memcpy(&ret, cache.line[s][u].data + b, len);
	return ret;
}
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	int tag = paddr / (cache.B * cache.S);
	int s = (paddr / cache.B) % cache.S;
	int b = paddr % cache.B;
	if(b+len>cache.B) {
		for(int i=0;i<len;i++)
			cache_write(paddr+i, 1, (uint32_t)((uint8_t *)&data)[i]);
		return;
	}
	int u = -1;
	for(int i=0;i<cache.E;i++) {
		if(cache.line[s][i].v && cache.line[s][i].tag == tag) {
			u = i;
			break;
		}
	}
	for(int i=0;i<cache.E;i++)
		if(i != u && cache.line[s][i].v)
			cache.line[s][i].t++;
	if(u == -1) {
		hw_mem_write(paddr, len, data);
	} else {
		cache.line[s][u].v = 1;
		cache.line[s][u].t = 0;
		cache.line[s][u].tag = tag;
		memcpy(cache.line[s][u].data + b, &data, len);
		hw_mem_write(paddr, len, data);
	}
}
```

部分 Testing

<img src="https://raw.githubusercontent.com/kouylty/nju-ics/master/notes/test3(1).png" style="zoom:50%;" alt="【若图片无法加载，详见https://github.com/kouylty/nju-ics/blob/master/notes/test3(1).png】">

至此，PA-3.1 完成。



#### PA-3.2 保护模式

在之前，我们的 $\tt{NEMU}$ 都是在“实模式”下运行，所有用户都对所有代码和程序有读写权限。但现代计算机并不是这样，现在的计算机都会划分特权等级。不同资源会对不同等级开放。计算机中一般有四个等级：ring 0~3，其中 ring 0 等级最高，一般对应操作系统内核；ring 3 最低，一般属于普通用户程序。为了实现这一特权等级制度，我们就要用到段寄存器（segment register）和保护模式（protect mode）来给内存中的资源做等级划分。这一阶段就是要实现这一个保护模式（尽管 $\tt{NEMU}$ 中没有等级划分）。

最早在 $\tt{8086}$ 年代，为了在不扩展寄存器位数的情况下扩展可寻址空间， intel 引入了段寄存器，同时设计了 `segment:offset` 的寻址方式。这样，物理地址的寻址就变成了 `paddr = (segment<<4) + offset`。段寄存器主要有 `CS`、`DS`、`SS` 和 `ES`，随着时代的发展，当初的这些段寄存器也被赋予了实现保护模式的任务。在保护模式下，程序给出的 $32$ 位地址不再直接是物理地址，而是相对一个基地址（base address）的偏移量（offset）。程序在访问内存时不仅会给出一个 $32$ 位地址，还会给出一个段选择符（segment selector）来获取基地址。段选择符存储在段寄存器中，同时段寄存器还会有对用户不可见的基地址等，用于建立保护模式。

```c
typedef struct {
	union { 	// visible to user, include selector
		uint16_t val;
		struct {
			uint32_t rpl :2;
			uint32_t ti :1;
			uint32_t index :13;
		};
	};
	struct { 	// invisible to user
		uint32_t base;
		uint32_t limit;
		uint32_t type :5;
		uint32_t privilege_level :2;
		uint32_t soft_use :1;
	};
}SegReg;
```

由段选择符获取基地址的过程通过查询段表来实现。段表在计算机启动时初始化并存储在内存中，段表中的每一个表项描述一个段的属性，包括基地址和长度等。为此我们定义数据类型 `SegDesc` 来模拟段表中的每一个表项，以及数据类型 `GDTR` 来记录段表的位置。

```c
typedef union SegmentDescriptor {
	struct
	{
		uint32_t limit_15_0 : 16;
		uint32_t base_15_0 : 16;
		uint32_t base_23_16 : 8;
		uint32_t type : 4;
		uint32_t segment_type : 1;
		uint32_t privilege_level : 2;
		uint32_t present : 1;
		uint32_t limit_19_16 : 4;
		uint32_t soft_use : 1;
		uint32_t operation_size : 1;
		uint32_t pad0 : 1;
		uint32_t granularity : 1;
		uint32_t base_31_24 : 8;
	};
	uint32_t val[2];
} SegDesc;
typedef struct {
	uint32_t limit :16;
	uint32_t base :32;
} GDTR;
```

**要求的内容：** 在GDTR中保存的段表首地址是什么形式？是线性地址（linear address），CPU 按 `GDTR.base + offset` 形成要访问的内存地址，这个地址在访问内存前是当作线性地址处理的。

说了这么多，我们还要控制计算机开启保护模式。这需要引入一个控制寄存器 `cr0`（control register）。这是一个 $32$ 位寄存器，其中 `pe` （protection enable）代表保护模式，`pg`（paging）代表分页，会在下一阶段用到。开启保护模式时，`pe` 就要置 $1$。

```c
typedef union {
	struct {
		uint32_t pe :1;
		uint32_t mp :1;
		uint32_t em :1;
		uint32_t ts :1;
		uint32_t et :1;
		uint32_t reserve :26;
		uint32_t pg :1;
	};
	uint32_t val;
}CR0;
```

计算机开启保护模式的过程是由内核控制。在计算机启动时，内核要先设置描述符表 `GDTR`，开启保护模式，装载并初始化各个段寄存器。在此之后才能开始运行用户程序。

**要求的内容：** $\tt{NEMU}$ 在什么时候进入保护模式？通过观察 `kernel.S`，我们发现有如下三条指令：

```assembly
movl    %cr0, %eax
orl     $0x1, %eax
movl    %eax, %cr0
```

这三条指令的功能是把控制寄存器 `cr0` 赋值成 $1$，这样就把 `cr0.pe` 置成了 $1$。更具体地，在 `mov_r2c_l` 后开启了保护模式。

我们提到的这些内核要做的事情，也要通过指令去完成。设置 `GDTR` 需要 `lgdt` 指令，这条指令接收两项按序连续存放的内存操作数，`m16&32`，把它们从内存中读出来再分别赋值到 `gdtr.limit` 和 `gdtr.base` 中。初始化段寄存器需要用 `ljmp` 和 `mov` 指令，其中 `ljmp` 在 $\tt{8086}$ 手册中没有，但其实这就是 `jmp_far` 指令。`mov` 指令要单独实现支持包含段寄存器和控制寄存器的移动。

```c
make_instr_func(lgdt) {
    int len = 1;
    OPERAND m;
    m.data_size = 16;
    len += modrm_rm(eip + 1, &m);
    operand_read(&m);
    cpu.gdtr.limit = m.val;
    m.data_size = 32;
    m.addr += 2;
    operand_read(&m);
    cpu.gdtr.base = m.val;
    print_asm_1("lgdt", "", len, &m);
    return len;
}
```

```c
make_instr_func(mov_rm2s_w) {
        int len = 1;
        opr_src.data_size = data_size;
        opr_dest.data_size = 16;
        len += modrm_r_rm(eip + 1, &opr_dest, &opr_src);
        operand_read(&opr_src);
        opr_dest.type = OPR_SREG;
        opr_dest.val = opr_src.val;
        operand_write(&opr_dest);
        load_sreg(opr_dest.addr);
        print_asm_2("mov", "s", len, &opr_src, &opr_dest);
        return len;
}

make_instr_func(mov_r2c_l) {
        int len = 1;
        opr_dest.data_size = 32;
        opr_src.data_size = 32;
        len += modrm_r_rm(eip + 1, &opr_src, &opr_dest);
        operand_read(&opr_src);
        opr_dest.type = OPR_CREG;
        opr_dest.val = opr_src.val;
        operand_write(&opr_dest);
        print_asm_2("mov", "c", len, &opr_src, &opr_dest);
        return len;
}
```

其中 `load_sreg` 函数就是把段寄存器中隐藏的 `base` 等部分加载出来，为后续访存做准备。需要注意的是，第 $7$ 行设置 `opr_dest.type = OPR_SREG` 必须在 `modrm` 函数后，这是因为在 `modrm` 函数中我们会先修改 `opr_dest.type = OPR_REG` 以便在内存中读取，会覆盖掉在这之前修改的 `type` 值。这个小 bug 卡了我很长时间，特别在此记录一下。

把初始化的操作都做好之后，就开始运行用户程序了。在这里要做的就是把保护模式下的虚拟地址转化成线性地址，按照基地址加上偏移量的方式计算即可。

```c
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	uint32_t base = cpu.segReg[sreg].base;
	return offset + base;
}

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
```

至此，PA-3.2 完成。



#### PA-3.3 分页机制和虚拟地址转换

计算机中会有许多进程同时运行，它们都要访问内存。在不同进程读写同一块内存区域时，就有可能发生冲突倒是错误。为了避免这种冲突，计算机里实行分页机制（paging）。给每个进程划分一个内存页作为进程私有的内存区域，同时设置读写和共享权限。分页后，每个进程都在自己的页中访存，而此时程序提供的地址就不再是直接在主存上的物理地址，而是一个虚拟地址（virtual address），要先转换为物理地址。

以上是整体思想，接下来我们一步一步看。首先是给每个进程分配自己的内存页。在我们的 $\tt{NEMU}$ 中，每加载一个 ELF 文件就新建（fork）了一个进程，我们要给每个 ELF 文件动态分配一块内存空间作为它的内存页。

```c
uint32_t *bp = (uint32_t *)mm_malloc(ph->p_vaddr, ph->p_memsz);
memcpy((void *)bp, (void *)elf + ph->p_offset, ph->p_filesz);
memset((void *)(bp + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
```

在这之后，就是要把虚拟地址翻译成物理地址。一般情况下，把 $32$ 位虚拟地址分为两部分：低 $12$ 位的偏虚拟页面移（virtual page offset, VPO）和高 $20$ 位的虚拟页号（virtual page number, VPN）。在翻译过程中，我们先根据 VPN 查询到对应的 $20$ 位物理页号（physical page number, PPN），再把 VPO 原位迁移成低 $12$ 位上的物理页面偏移（physical page offset, PPO），两者整合成最终的物理地址。

在 VPN 查询到 PPN 的过程中，就需要用到页表（page table）。页表就是一个页表条目（page table entry, PTE）数组，里面存储着对应的 PPN、有效位、读写权限位、记录修改的脏位（dirty bit）等。我们要做的就是用 VPN 找到对应的 PTE 即可。但是因为虚拟页的数量很多，存储页表很占空间，每次查询页表也会耗费大量时间，所以我们可以把页表分级。在 $\tt{NEMU}$ 中我们分成了两级页表，我们把 VPN 的 $20$ 位拆分成了高 $10$ 位的 `dir` 和低 $10$ 位的 `page`。使用 `dir` 查询第一季页表，第一级页表（page directory）其实是第二级页表的目录，找到之后再用 `page` 查询第一级页表表项（page directory entry, PDE）中对应的第二级页表，最终找到对应的 PPN。

还有一个小问题，就是我们还要记录页表的物理地址用于访问页表，这其实是记录在一个控制寄存器 `cr3` 中，记作 `cr3.dir`。

```c
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
	uint32_t dir = laddr >> 22 & 0x3ff;
	uint32_t page = laddr >> 12 & 0x3ff;
	uint32_t offset = laddr & 0xfff;
	PDE *pde = (PDE *)(hw_mem + (cpu.cr3.dir << 12) + dir * 4);
	assert(pde->present == 1);
	PTE *pte = (PTE *)(hw_mem + (pde->page_frame << 12) + page *4);
	assert(pte->present == 1);
	return (pte->page_frame << 12) | offset;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
```

当然，页表长度是有限的。如果计算机中进程很多，新建的虚拟页很多，查询时页表中就可能没有对应表项，此时就会出现缺页（page fault），计算机就会回到磁盘中取出对应的页，然后替换页表中的一项并把新取出来的页放到页表里。替换过程与缓存类似，也是 LRU。尽管在 $\tt{NEMU}$ 中保证了不会缺页，但缺页消耗的时间是很多的，最好要避免缺页的情况发生。

为了让页表访问速度更快，实际上我们也可以为页表做一组缓存，这就是快表（translation lookaside buffer, TLB）。TLB 的工作机制与主存缓存类似，把虚拟地址中的 VPN 拆分成了 TLB 标记 `TLBT` 和 TLB 索引 `TLBI`，要访问页表条目时可以先访问 TLB，就可以加快页表查询效率。

最后，我们来稍微看看 kernel 中提到的 `mm_malloc` 函数具体做了哪些事。除了为每个 ELF 文件分配内存，它还要为页分配做工作。具体的，`mm_malloc` 要先计算 VPN 和 VPO，计算出要映射的页的数量，再从物理内存中拿到页帧，为每一个内存页新建自己的 PTE（包括记录权限、修改有效位、记录 PPN 等），最后在把 PTE 加入页表中。

至此，PA-3.3完成。



PA-3 撒花！ (╹ڡ╹ )
