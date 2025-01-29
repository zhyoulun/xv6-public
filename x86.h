// Routines to let C code use special x86 instructions.

// 从指定的 I/O 端口读取一个字节数据
// 在操作系统内核开发或底层驱动程序开发中，这种代码通常用于与硬件直接交互。
static inline uchar
inb(ushort port)
{
  uchar data;

  asm volatile("in %1,%0" : "=a" (data) : "d" (port));
  return data;
}

// 用于从 I/O 端口读取多个 32 位（即 4 字节）数据的函数，通常用于低级设备驱动开发
static inline void
insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

// 向指定的 I/O 端口写入一个字节的数据
static inline void
outb(ushort port, uchar data)
{
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

// 用于向指定的 I/O 端口写入一个 16 位数据
static inline void
outw(ushort port, ushort data)
{
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

// 用于通过指定的 I/O 端口批量写入数据，每次写入 4 字节（即一个 32 位整数）
static inline void
outsl(int port, const void *addr, int cnt)
{
  asm volatile("cld; rep outsl" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}

// 使用 stosb 指令将字节值 data 存储到内存区域 addr，重复执行 cnt 次。每次存储 1 字节。
static inline void
stosb(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosb" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}

// 使用 stosl 指令将字值 data 存储到内存区域 addr，重复执行 cnt 次。每次存储 4 字节。
static inline void
stosl(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosl" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}

struct segdesc;

// 用于加载 GDT（Global Descriptor Table） 的基址和界限。lgdt 是 x86 指令集中的一条指令，用于加载全局描述符表的地址和大小。
static inline void
lgdt(struct segdesc *p, int size)
{
  volatile ushort pd[3];

  pd[0] = size-1;
  pd[1] = (uint)p;
  pd[2] = (uint)p >> 16;

  asm volatile("lgdt (%0)" : : "r" (pd));
}

struct gatedesc;

// 用于设置 IDT（Interrupt Descriptor Table） 的基址和界限。lidt 是 x86 指令集中的一条指令，用于加载中断描述符表的地址和大小。
static inline void
lidt(struct gatedesc *p, int size)
{
  volatile ushort pd[3];

  pd[0] = size-1;
  pd[1] = (uint)p;
  pd[2] = (uint)p >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
}

// 用于将一个段选择子加载到 x86 架构的 Task Register（任务寄存器）中。ltr 是 x86 指令集中的一条指令，用于设置任务寄存器。
static inline void
ltr(ushort sel)
{
  asm volatile("ltr %0" : : "r" (sel));
}

// 用于读取当前 CPU 状态寄存器中的 EFLAGS 寄存器值。EFLAGS 寄存器包含了控制程序执行的一些标志位，如中断标志、零标志、溢出标志等。
static inline uint
readeflags(void)
{
  uint eflags;
  asm volatile("pushfl; popl %0" : "=r" (eflags));
  return eflags;
}

// 用于将一个值加载到 x86 架构的 GS 段寄存器中
static inline void
loadgs(ushort v)
{
  asm volatile("movw %0, %%gs" : : "r" (v));
}

// close interrupt flag
static inline void
cli(void)
{
  asm volatile("cli");
}


// 用于执行 x86 指令 sti，它的作用是开启中断（Set Interrupt Flag）
static inline void
sti(void)
{
  asm volatile("sti");
}

// 用于原子性地交换变量的值
// 这种操作在操作系统和多线程编程中常用于实现同步原语（例如锁）
static inline uint
xchg(volatile uint *addr, uint newval)
{
  uint result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}

// 用于将指定的值写入到控制寄存器 CR2 中
static inline uint
rcr2(void)
{
  uint val;
  asm volatile("movl %%cr2,%0" : "=r" (val));
  return val;
}

// 用于将指定的值写入到控制寄存器 CR3 中
static inline void
lcr3(uint val)
{
  asm volatile("movl %0,%%cr3" : : "r" (val));
}

//PAGEBREAK: 36
// Layout of the trap frame built on the stack by the
// hardware and by trapasm.S, and passed to trap().
struct trapframe {
  // registers as pushed by pusha
  uint edi;
  uint esi;
  uint ebp;
  uint oesp;      // useless & ignored
  uint ebx;
  uint edx;
  uint ecx;
  uint eax;

  // rest of trap frame
  ushort gs;
  ushort padding1;
  ushort fs;
  ushort padding2;
  ushort es;
  ushort padding3;
  ushort ds;
  ushort padding4;
  uint trapno;

  // below here defined by x86 hardware
  uint err;
  uint eip;
  ushort cs;
  ushort padding5;
  uint eflags;

  // below here only when crossing rings, such as from user to kernel
  uint esp;
  ushort ss;
  ushort padding6;
};
