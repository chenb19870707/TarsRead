# 原子计数类tc_atomic

实现与linux实现方式相同，均为调用xaddl汇编。

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


### tc_atomic.h

``` c++
#ifndef __TC_ATOMIC_H
#define __TC_ATOMIC_H

#include <stdint.h>

namespace tars
{

/////////////////////////////////////////////////
/** 
 * @file  tc_atomic.h 
 * @brief  原子计数类. 
 */           

__BEGIN_DECLS 

#define TARS_LOCK "lock ; "

typedef struct { volatile int counter; } tars_atomic_t;

#define tars_atomic_read(v)        ((v)->counter)

#define tars_atomic_set(v,i)       (((v)->counter) = (i))

__END_DECLS

/**
 * @brief 原子操作类,对int做原子操作
 */
class TC_Atomic
{
public:

    /**
     * 原子类型
     */
    typedef int atomic_type;

    /**
     * @brief 构造函数,初始化为0 
     */
    TC_Atomic(atomic_type at = 0)
    {
        set(at);
    }

    TC_Atomic& operator++()
    {
        inc();
        return *this;
    }

    TC_Atomic& operator--()
    {
        dec();
        return *this;
    }

    operator atomic_type() const
    {
        return get();
    }

    TC_Atomic& operator+=(atomic_type n)
    {
        add(n);
        return *this;
    }

    TC_Atomic& operator-=(atomic_type n)
    {
        sub(n);
        return *this;
    }

    TC_Atomic& operator=(atomic_type n)
    {
        set(n);
        return *this;
    }

    /**
     * @brief 获取值
     *
     * @return int
     */
    atomic_type get() const           { return _value.counter; }

    /**
     * @brief 添加
     * @param i
     *
     * @return int
     */
    atomic_type add(atomic_type i)    { return add_and_return(i); }

    /**
     * @brief 减少
     * @param i
     *
     * @return int
     */
    atomic_type sub(atomic_type i)    { return add_and_return(-i); }

    /**
     * @brief 自加1
     *
     * @return int
     */
    atomic_type inc()               { return add(1); }

    /**
     * @brief 自减1
     */
    atomic_type dec()               { return sub(1); }

    /**
     * @brief 自加1
     *
     * @return void
     */
    void inc_fast()
    {
        __asm__ __volatile__(
            TARS_LOCK "incl %0"
            :"=m" (_value.counter)
            :"m" (_value.counter));
    }

    /**
     * @brief 自减1
     * Atomically decrements @_value by 1 and returns true if the
     * result is 0, or false for all other
     */
    bool dec_and_test()
    {
        unsigned char c;

        __asm__ __volatile__(
            TARS_LOCK "decl %0; sete %1"
            :"=m" (_value.counter), "=qm" (c)
            :"m" (_value.counter) : "memory");

        return c != 0;
    }

    /**
     * @brief 设置值
     */
    atomic_type set(atomic_type i)
    {
        _value.counter = i;

        return i;
    }

protected:

    /**
     * @brief 增加并返回值
     */
    int add_and_return(int i)
    {
        /* Modern 486+ processor */
        int __i = i;
        __asm__ __volatile__(
            TARS_LOCK "xaddl %0, %1;"
            :"=r"(i)
            :"m"(_value.counter), "0"(i));
        return i + __i;
    }

protected:

    /**
     * 值
     */
    tars_atomic_t    _value;
};

}

#endif
```