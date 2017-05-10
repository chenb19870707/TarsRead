# 原子计数类tc_atomic

实现与linux实现方式相同，均为调用xaddl汇编。

### code：[tc_atomic.h](https://github.com/Tencent/Tars/blob/master/cpp/util/include/util/tc_atomic.h)

## 知识点

### 扩充编译
``` c++
__BEGIN_DECLS
.....
.....
__END_DECLS
```
扩充编译是，这段部分按照BEGIN  end 之间的进行编译。

### 汇编实现原子增加并返回值
试用条件:486以上CPU   

#### 1.GCC在C语言中内嵌汇编 
	`asm __volatile__(汇编代码)`
####  2.lock 指令前缀

在所有的 ***X86 CPU*** 上都具有锁定一个特定内存地址的能力，当这个特定内存地址被锁定后，它就可以阻止其他的系统总线读取或修改这个内存地址。这种能力是通过 LOCK 指令前缀再加上下面的汇编指令来实现的。当使用 LOCK 指令前缀时，它会使 CPU 宣告一个 LOCK# 信号，这样就能确保在多处理器系统或多线程竞争的环境下互斥地使用这个内存地址。当指令执行完毕，这个锁定动作也就会消失。

####  3.xaddl 指令

Exchanges the first operand (destination operand) with the second operand (source operand), then loads the sum of the two values into the destination operand. The destination operand can be a register or a memory location; the source operand is a register.
```
TEMP ← SRC + DEST;
SRC ← DEST;
DEST ← TEMP;
```

测试：
``` c++
int add_and_return(int i)
{
    /* Modern 486+ processor */
    int __i = i;
    printf(" pre xaddl i = %d,__i = %d,_value.counter=%d\n",i,__i,_value.counter);
    __asm__ __volatile__(
        TARS_LOCK "xaddl %0, %1;"
        :"=r"(i)
        :"m"(_value.counter), "0"(i));

    printf("after xaddl i = %d,__i = %d,i+__i=%d,_value.counter=%d\n",i,__i,i+__i,_value.counter);
    return i + __i;
}
```

结果：
```
pre xaddl i = 1,__i = 1,_value.counter=10
after xaddl i = 10,__i = 1,i+__i=11,_value.counter=11
```
**xaddl 交换i和value.counter,并把和写进value.counter**
