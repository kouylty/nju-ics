### PA-1 数据的表示、存储和运算：笔记

kouylty



写在最前面：这个是本人自己写 PA 时写的笔记，有提到一些知识点，内容也比较自由。对于实验手册中要求的内容，其中都有涵盖，会在段首写上 “**要求的内容**” 标志一下。



#### PA-1.1 数据在计算机内的存储

计算机内部，有一个存储器体系结构，从顶层到底层大致分别包括：寄存器（register）、缓存（cache）、内存（memory）、硬盘（disk）等。在这一部分，我们主要讨论寄存器和主存。

首先是寄存器，在我们的 $\tt{NEMU}$ 中，在 ```./include/nemu/cpu/reg.h``` 里，我们想用一个结构体 ```CPU_STATE``` 模拟寄存器。为了模拟的方便，我们想让 ```cpu.eax``` 和 ```cpu.gpr[0]._32``` 访问到的是同一寄存器，还有 ```cpu.al```（```cpu.eax``` 的低 $16$ 位）和 ```cpu.gpr[0]._16``` 访问到的是同一寄存器，等等。因此，我们把三处 ```struct``` 改成 ```union```，写出以下代码。

```c
union {
    union {
        union {
            uint32_t _32;
            uint16_t _16;
            uint8_t _8[2];
        };
        uint32_t val;
    }gpr[8];
    struct { // do not change the order of the registers
        uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    };
};
```

**要求的内容：**为什么把 ```struct``` 改成 ```union``` 就可以了呢？这是因为，```struct``` 里的元素在内存里是线性存储的，并且在数据对齐（alignment）的基础上保持连续，这样，```gpr[0]._32```、```gpr[0]._16```、```gpr[0].val```、```eax``` 等访问到的就是不同的内存地址。但是我们想让其中对应的变量访问到的是同一个内存地址，也就是说，我们想让我们的数据在内存里只存一份，然后让这些元素并列（parallel）存储。这就需要用到联合，```union``` 让 ```gpr[]``` 与每一个寄存器并列，实现一一对应。

另外，寄存器中还有重要的标志位寄存器 ```eflags```，这些标志位有辅助 ALU 计算、记录系统状态等功能，在后面的 PA 中会具体涉及。

其次是内存，在 $\tt{NEMU}$ 中，我们想模拟一个 $128\mathrm{MB}$ 的内存。内存抽象地讲就是一个很大的数组，因此我们定义 ```uint8_t hw_mem[MEM_SIZE_B];```，其中 ```MEM_SIZE_B``` 等于 $128\times1024\times1024$。内存地址分为物理地址（physical address）、线性地址（linear address）、虚拟地址（virtual address）。现阶段，我们只考虑物理地址，其他内存管理方式在 PA-3 中再涉及。因为我们是计算机的模拟器，因此访存需要使用软件接口实现。在此我们编写了物理内存的读写接口。

```c
uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	ret = hw_mem_read(paddr, len);
	return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data)
{
	hw_mem_write(paddr, len, data);
}
```

至此，PA-1.1 完成。



#### PA-1.2 整数的表示、存储和运算

在计算机中，整数分为有符号整数和无符号整数。无符号整数通常用补码表示，补码在运算中与无符号数的运算规则一致。因此只用实现无符号数的 ALU 计算即可。

此外，我们说在 ALU 计算时，cpu 会同步设置若干运算标志位，统一存在 ```eflags``` 寄存器中。标志位主要有以下几个。

```ZF```（Zero Flag）：如果运算结果为 $0$，则设置，否则清零。

```SF```（Sign Flag）：如果运算结果为负数，则设置，否则清零。

```PF```（Parity Flag）：如果运算结果的低 $8$ 位中含有偶数个 $1$，则设置，否则清零。

```OF```（Overflow Flag）：如果运算结果越界（超过该数据类型的表示范围），则设置，否则清零。

```CF```（Carry Flag）：如果运算结果的最高位有加法进位或减法退位，则设置，否则清零。

```AF```（Adjust Flag）：如果低半字节（第 $3$ 位到第 $4$ 位）产生了进位或退位，则设置，否则清零。

在这些标志位中，```ZF```、```SF``` 和 ```PF``` 在不同运算中设置方法相同。```OF``` 和 ```CF``` 因为在不同的运算中定义有细微差别，因此在实现中一般不同。```AF``` 用于十进制计算和 BCD 码转换，在此 PA 中不涉及。

此外，数据在 $\tt{NEMU}$ 中采取小端（little endian）方式存储，即低位（least significant）字节在前，高位（most significant）字节在后。

好的，接下来我们来深入讨论每种运算。首先是整数的加减法运算。一共有四种，分别是 ```add```、```adc```（add with carry）、```sub```、```sbb```（sub with borrow）。有了这四种运算，我们可以对任意位数的整数进行加减。对于加法，当结果小于加数的时候，设置 ```CF```，这很容易理解。特别的，在 ```adc``` 中，如果 ```CF==1```，那么还有一种可能是结果等于加数。考虑什么时候设置 ```OF```，如果两个正数加出一个负数，或者两个负数加出一个正数，那就是溢出了，因此我们只要判断结果和两个加数的符号即可。对于减法，当被减数 ```dest``` 小于减数 ```src``` 时会发生最高位退位，要设置 ```CF```。当然在 ```sbb``` 中还有一种特殊情况是 ```dest==src``` 同时有之前的退位，即 ```CF==1```。设置 ```OF``` 与加法类似的两种情况，判断符号位即可。

另外，因为整型有不同位数，即 ```data_size```，计算过程中要考虑截断，高位补零。

```c
uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = src + dest;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	cpu.eflags.CF = (s < dest);
	cpu.eflags.OF = ((dest>>(data_size-1))==0 && (src>>(data_size-1))==0 && (s>>(data_size-1))==1) || ((dest>>(data_size-1))==1 && (src>>(data_size-1))==1 && (s>>(data_size-1))==0);
	return s;
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = src + dest + cpu.eflags.CF;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	cpu.eflags.CF = (s < dest) || (cpu.eflags.CF && s == dest);
	cpu.eflags.OF = ((dest>>(data_size-1))==0 && (src>>(data_size-1))==0 && (s>>(data_size-1))==1) || ((dest>>(data_size-1))==1 && (src>>(data_size-1))==1 && (s>>(data_size-1))==0);
	return s;
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = dest - src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	cpu.eflags.CF = (dest < src);
	cpu.eflags.OF = ((dest>>(data_size-1))==1 && (src>>(data_size-1))==0 && (s>>(data_size-1))==0) || ((dest>>(data_size-1))==0 && (src>>(data_size-1))==1 && (s>>(data_size-1))==1);
	return s;
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
	dest &= (0xffffffff >> (32-data_size));
	src &= (0xffffffff >> (32-data_size));
	uint32_t s = dest - src - cpu.eflags.CF;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	cpu.eflags.CF = (dest < src) || (cpu.eflags.CF && dest == src);
	cpu.eflags.OF = ((dest>>(data_size-1))==1 && (src>>(data_size-1))==0 && (s>>(data_size-1))==0) || ((dest>>(data_size-1))==0 && (src>>(data_size-1))==1 && (s>>(data_size-1))==1);
	return s;
}
```

然后是逻辑运算和移位运算。逻辑运算（```and```、```or```、```xor```）很简单，且不涉及进位和溢出，在此不赘述。主要看移位运算，分为左移和右移，右移又分为逻辑右移 ```shr```（logical shift right）和算术右移 ```sar```（arithmetic shift right）。逻辑右移是高位补零，算数右移是高位补符号位，具体实现中先与符号位掩码做或，再截断即可。对这三种移位运算，都无需设置 ```OF```（在 manual 中有一种只移一位的命令，需要设置 ```OF```，在此 PA 中不考虑），要将 ```CF``` 设置成最后一个被移出的位。

```c
uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint32_t s = dest << src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = (dest >> (data_size - src)) & 0x1;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint32_t s = dest >> src;
	s &= (0xffffffff >> (32-data_size));
	cpu.eflags.CF = (dest >> (src - 1)) & 0x1;
	cpu.eflags.ZF = (s == 0);
	cpu.eflags.SF = (s >> (data_size - 1));
	cpu.eflags.PF = __builtin_parity(s&255)%2==0;
	return s;
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
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
}
```

最后是乘除运算，分为有符号乘除和无符号乘除。对乘法运算，乘数位数为 ```data_size```，那结果的位数就是 ```data_size*2```。只需设置 ```CF``` 和 ```OF```，又因为乘法不涉及进位，```CF``` 的设置方法与 ```OF``` 相同。我们用的是 ```int64_t``` 存储结果，可以直接通过比较结果与该数据范围下的最大值和最小值来判断是否溢出。在 8086 中，执行除法时会同时给出商和余数，因此我们新增了 ```mod``` 和 ```imod``` 函数来剥离运算。除法不需要设置符号位。

```c
uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
	src &= (0xffffffff >> (32-data_size));
	dest &= (0xffffffff >> (32-data_size));
	uint64_t s = (uint64_t)src * (uint64_t)dest;
	s &= (0xffffffffffffffff >> (64-data_size-data_size));
	uint64_t mx = 0xffffffff >> (32-data_size);
	cpu.eflags.OF = (s > mx);
	cpu.eflags.CF = (s > mx);
	return s;
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
	int64_t s = (int64_t)src * (int64_t)dest;
	int64_t mx = (1LL << data_size) - 1, mn = -(1LL << data_size);
	cpu.eflags.OF = (s > mx || s < mn);
	cpu.eflags.CF = (s > mx || s < mn);
	return s;
}

uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
	assert(src!=0);
	uint32_t s = dest / src;
	s &= 0xffffffff >> (32-data_size);
	return s;
}

int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
	assert(src!=0);
	int32_t s = dest / src;
	return s;
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
	uint32_t s = dest % src;
	return s;
}

int32_t alu_imod(int64_t src, int64_t dest)
{
	int32_t s = dest % src;
	return s;
}
```

至此，PA-1.2 完成。



#### PA-1.3 浮点数的表示和运算

在 IEEE754 标准中，单精度浮点数 ```float```（$32$ 位）由 $1$ 位符号位 ```s```（sign）、$8$ 位阶码位 ```exp```（exponent）和 $23$ 位尾数位 ```frac```（fraction）从高到低组成。阶码采取移码表示，偏移量（bias）一般取 $-127$。根据阶码和尾数，分成四种解释方式。

第一种，$\mathrm{exp}\neq0$ 且 $\mathrm{exp}\neq255$：规格化（normalized）浮点数，计算方式为 $\mathrm{s}\times 1.\mathrm{frac}\times2^{\mathrm{exp-127}}$。

第二种，$\mathrm{exp=0}$：非规格化（denormalized）浮点数，计算方式为 $\mathrm{s}\times0.\mathrm{frac}\times2^{-126}$。

第三种，$\mathrm{exp}=255$ 且 $\mathrm{frac}=0$：无穷大 ```inf```。

第四种，$\mathrm{exp}=255$ 且 $\mathrm{frac}\neq0$：Not a Number ```nan```。

其中，设置非规格化数目的是提高 $0$ 附近的表示精度，但这样对 $0$ 本身来说就会有正零和负零两种形式，都是合法的，编程时要特别注意。同时又因为有 ```inf``` 和 ```nan```，因此浮点数计算中并不会报除零错误等，而是表示为 ```inf``` 或 ```nan```。这种浮点数表示方法还有一种好处，就是浮点数做大小比较的规则就跟带符号整数一致。

我们的 $\tt{NEMU}$ 只考虑单精度浮点数，用 ```uint32_t``` 表达一个浮点数。接下来我们来模拟浮点数的四则运算。为了保证运算时的精度，我们在运算过程中会将尾数扩展 $3$ 位（称为 GRS），最后再进行舍入（round to even）。

首先是加减，使用类似于科学计数法的计算方法，先对阶，保证指数相同，前面的系数（significand）再进行加法。对阶过程要注意规格化数和非规格化数的偏移量不同，系数加法遵循无符号数加法规则。

```c
uint32_t internal_float_add(uint32_t b, uint32_t a)
{
	// corner cases
	int i = 0;
	for (; i < sizeof(corner_add) / sizeof(CORNER_CASE_RULE); i++)
		if (a == corner_add[i].a && b == corner_add[i].b)
			return corner_add[i].res;
	if (a == P_ZERO_F || a == N_ZERO_F) return b;
	if (b == P_ZERO_F || b == N_ZERO_F) return a;
	FLOAT f, fa, fb;
	fa.val = a, fb.val = b;
	// infity, NaN
	if (fa.exponent == 0xff) return a;
	if (fb.exponent == 0xff) return b;
	if (fa.exponent > fb.exponent) fa.val = b, fb.val = a;
	uint32_t sig_a = fa.fraction, sig_b = fb.fraction, sig_res = 0;
	if (fa.exponent != 0) sig_a |= 0x800000; // the hidden 1
	if (fb.exponent != 0) sig_b |= 0x800000; // the hidden 1
	uint32_t shift = fb.exponent - fa.exponent;
	if(shift>0 && fa.exponent==0) shift--;
	sig_a = (sig_a << 3); // guard, round, sticky
	sig_b = (sig_b << 3);
	uint32_t sticky = 0;
	while (shift--)
	{
		sticky = sticky | (sig_a & 0x1);
		sig_a >>= 1;
		sig_a |= sticky;
	}
	if (fa.sign) sig_a *= -1;
	if (fb.sign) sig_b *= -1;
	sig_res = sig_a + sig_b;
	if (sign(sig_res)) f.sign = 1, sig_res *= -1;
	else f.sign = 0;
	uint32_t exp_res = fb.exponent;
	return internal_normalize(f.sign, exp_res, sig_res);
}
```

加法过程中，我们增加了尾数精度，另外尾数加法最高位也有可能产生进位，因此最初结果不合标准，我们要在此做调整，就是我们的 ```internal_normalize``` 函数。

这个函数的主要功能是调整尾数的位数，同时将阶码对应修改。注意把 ```inf``` 和 ```nan``` 单独摘出。

```c
inline uint32_t internal_normalize(uint32_t sign, int32_t exp, uint64_t sig_grs)
{
	bool overflow = false; // true if the result is INFINITY or 0 during normalize
	sign &= 1;
	if ((sig_grs >> (23 + 3)) > 1 || exp < 0)
	{
        // normalize towards right
		while ((((sig_grs >> (23 + 3)) > 1) && exp < 0xff) // condition 1
			   || (sig_grs > 0x04 && exp < 0))			   // condition 2
		{
			uint8_t sticky = sig_grs&1;
			sig_grs >>= 1;
			sig_grs |= sticky;
			exp++;
		}
		if (exp >= 0xff) //  assign the number to infinity
		{
			overflow = true;
			return 0x7f800000 | (sign << 31);
		}
		else if (exp == 0) // denormal
		{
			uint8_t sticky = sig_grs&1;
			sig_grs >>= 1;
			sig_grs |= sticky;
		}
		else if (exp < 0) // assign the number to zero
		{
			overflow = true;
			return sign << 31;
		}
	}
	else if (((sig_grs >> (23 + 3)) == 0) && exp > 0)
	{
		// normalize toward left
		while (((sig_grs >> (23 + 3)) == 0) && exp > 0)
		{
			sig_grs <<= 1;
			exp--;
		}
		if (exp == 0) // denormal
		{
			uint8_t sticky = sig_grs&1;
			sig_grs >>= 1;
			sig_grs |= sticky;
		}
	}
	else if (exp == 0 && sig_grs >> (23 + 3) == 1) 
		exp++;
	if (!overflow)
	{
		// round up and remove the GRS bits
		uint32_t grs = sig_grs&7;
		sig_grs >>= 3;
		if (grs>4 || (grs==4 && (sig_grs&1)))
		{
			sig_grs++;
			if ((sig_grs >> 24) & 1)
			{
				sig_grs >>= 1;
				exp++;
				if (exp >= 0xff)
				{
					overflow = true;
					return 0x7f800000 | (sign << 31);
				}
			}
		}
	}
	FLOAT f;
	f.sign = sign;
	f.exponent = (uint32_t)(exp & 0xff);
	f.fraction = sig_grs & 0x7fffff; // here only the lowest 23 bits are kept
	return f.val;
}
```

对于减法，我们做的是把被减数取相反数，然后执行加法，在此不做赘述。

**要求的内容：**一个规格化数与一个非规格化数加减，会不会出现阶码上（下）溢？一定是不会的。直观地想，非规格化数太小太小了，肯定不会让阶码发生上溢。而对于下溢，最大的非规格化数（```0x007fffff```）也比最小的规格化数（```0x00800000```）来的小，两者相减后等于 ```0x00000001```，不足以让阶码下溢。

加减过后就是乘除。乘法直接省去了对阶的操作，只要让两个阶数相加就行，注意还要减去一个偏移量。但乘法的实现细节比较多，也花了较多时间，在此稍微讲一下。先做完乘 $0$ 和乘 ```inf``` 的情况，再来看剩下的。如果是两个非规格化数相乘，阶码必定下溢，此时直接返回 $0$ 即可。最后就剩下乘数中有规格化数的情况，规格化的尾数部分有一个隐式的 $1$ 在小数点前，这个要补上。又因为我们是用 ```uint32_t``` 模拟尾数，因此在做尾数乘法时我们实际上做的是整数乘法，这样对阶数来说就要先减掉一个 $23$（这是由于尾数有 $23$ 位），然后在最后 ```normalize``` 函数中再调整回小数点前只有一个 $1$ 的形式。不管是有几个规格化数，都是减掉一个 $23$ 就好，这个数学推导一下不难。还有一点是关于 GRS 位的事情，因为乘法本身的精度其实远远超出我们的要求的（结果的尾数一开始都是 $46$ 位），所以我们其实没有必要扩展精度，最后预留好末尾三个零即可。

```c
uint32_t internal_float_mul(uint32_t b, uint32_t a)
{
	// corner cases
	int i = 0;
	for (; i < sizeof(corner_mul) / sizeof(CORNER_CASE_RULE); i++)
		if (a == corner_mul[i].a && b == corner_mul[i].b)
			return corner_mul[i].res;
	if (a == P_NAN_F || a == N_NAN_F || b == P_NAN_F || b == N_NAN_F)
		return a == P_NAN_F || b == P_NAN_F ? P_NAN_F : N_NAN_F;
	FLOAT fa, fb, f;
	fa.val = a, fb.val = b;
	f.sign = fa.sign ^ fb.sign;
	if (a == P_ZERO_F || a == N_ZERO_F)
		return f.sign ? N_ZERO_F : P_ZERO_F;
	if (b == P_ZERO_F || b == N_ZERO_F)
		return f.sign ? N_ZERO_F : P_ZERO_F;
	if (a == P_INF_F || a == N_INF_F)
		return f.sign ? N_INF_F : P_INF_F;
	if (b == P_INF_F || b == N_INF_F)
		return f.sign ? N_INF_F : P_INF_F;
	uint64_t sig_a = fa.fraction, sig_b = fb.fraction, sig_res = 0;
	if(fa.exponent==0 && fb.exponent==0)
		return f.sign ? N_ZERO_F : P_ZERO_F;
	int32_t exp_res = fa.exponent + fb.exponent - 127 - 23;
	if (fa.exponent != 0) sig_a |= 0x800000;
	else exp_res++;
	if (fb.exponent != 0) sig_b |= 0x800000;
	else exp_res++;
	sig_res = sig_a * sig_b; // 24b * 24b
	sig_res <<= 3; // leave space for GRS bits
	return internal_normalize(f.sign, exp_res, sig_res);
}
```

对于除法其实类似，只是在保证精度上做了一些调整。我们先尽可能扩展被除数尾数的位数到 $64$，再通过右移舍弃除数尾数末尾的 $0$ 来缩短除数尾数的位数，这样尾数相除时的精度就可以保证，然后最后再把结果调整回去就好。

```c
uint32_t internal_float_div(uint32_t b, uint32_t a)
{
	int i = 0;
	for (; i < sizeof(corner_div) / sizeof(CORNER_CASE_RULE); i++)
	{
		if (a == corner_div[i].a && b == corner_div[i].b)
			return corner_div[i].res;
	}
	FLOAT f, fa, fb;
	fa.val = a;
	fb.val = b;
	if (a == P_NAN_F || a == N_NAN_F || b == P_NAN_F || b == N_NAN_F)
		return a == P_NAN_F || b == P_NAN_F ? P_NAN_F : N_NAN_F;
	if (a == P_INF_F || a == N_INF_F)
		return fa.sign ^ fb.sign ? N_INF_F : P_INF_F;
	if (b == P_INF_F || b == N_INF_F)
		return fa.sign ^ fb.sign ? N_ZERO_F : P_ZERO_F;
	if (b == P_ZERO_F || b == N_ZERO_F)
		return fa.sign ^ fb.sign ? N_INF_F : P_INF_F;
	if (a == P_ZERO_F || a == N_ZERO_F)
	{
		fa.sign = fa.sign ^ fb.sign;
		return fa.val;
	}
	f.sign = fa.sign ^ fb.sign;
	uint64_t sig_a = fa.fraction, sig_b = fb.fraction, sig_res = 0;
	if (fa.exponent != 0) sig_a |= 0x800000; // the hidden 1
	if (fb.exponent != 0) sig_b |= 0x800000; // the hidden 1
	// efforts to maintain the precision of the result
	int shift = 0;
	while (sig_a >> 63 == 0)
	{
		sig_a <<= 1;
		shift++;
	}
	while ((sig_b & 0x1) == 0)
	{
		sig_b >>= 1;
		shift++;
	}
	sig_res = sig_a / sig_b;
	if (fa.exponent == 0) fa.exponent++;
	if (fb.exponent == 0) fb.exponent++;
	uint32_t exp_res = fa.exponent - fb.exponent + 127 - (shift - 23 - 3);
	return internal_normalize(f.sign, exp_res, sig_res);
}
```

至此，PA-1.3 完成。



#### PA-1 Testing

<img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\test1(1).png" style="zoom:50%;" /> <img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\test1(2).png" style="zoom:50%;" /> <img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\test1(3).png" style="zoom:50%;" /> <img src="E:\NJU\计算机系统基础（苏丰汪亮）\__PA\pa_nju\notes\test1(4).png" style="zoom:50%;" />

PA-1 撒花！ (╹ڡ╹ )
