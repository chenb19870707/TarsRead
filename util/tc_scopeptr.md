# 智能指针类 TC_ScopedPtr

## 知识点

### 1.类函数指针

```
#include <iostream>
using namespace std;

class A
{
        typedef int (A::*func)() ;
        int func1()
        {
                return 1;
        }

public:
        func func2()
        {
                return &A::func1;
        }
};

int main()
{
        A* p = new A();
        cout << (p->func2)() << endl;
}

```

### 2. operator unspecified-bool-type() const

为了使只能指针使用上更接近原始指针，如下代码:
```
tc_scopte_ptr sp(new Something);

if(sp)
{
	// do something
}
else
{
	// do something
}

```

如何使用智能指针能自动转换成一个bool值呢？

解决的方法，实现很简单。 
C++有bool操作符。我们只需要针对智能指针，重载一下这个操作符
就OK了。
```
    #if defined(__SUNPRO_CC) && BOOST_WORKAROUND(__SUNPRO_CC, <= 0x530)

    operator bool () const
    {
        return px != 0;
    }

```

问题2： 这样安全嘛？    

	上面的方法解决了大部分应用的问题。可是，如果用户写出如下的代码呢？

	int i = sp;
	
这个时候，编译器并不会报错。事实上它在此自动对sp调用了bool(),将其转换成一个布尔值，
然后再将这个布尔值转换成一个整形值。所有的一些都在无形之中作了处理。这里的示例似乎并不能看出
这样的转换有什么危害。事实上这种在程序员并不知情时作的转换，已经使代码对程序员思想的表达发生
了变化，使用程序更不可读。当然在这种混乱的情况下，谁能保证一定不会出错呢？？？

问题3： 如何解决问题2？

	在C++标准4.12节中， 对指针与布尔值之间的关系有大致如下的描述： 
任何一个指针或成员指针
都可以转换成布尔值，空指针转换成false, 其它值转换成true.

	这样我们就可以利用成员指针来进行转换。 
通常成员指针不可能（至少不那种容易）被隐式转换
（转换的前提毕竟是类型要匹配，或可向上转换）。

```
// > 首先定义一个成员函数指针，在px不为空时，返回非空值。这样这个非空的成员函数指针就可以被解释为true
// > 反之被解释为false,达到重载bool相同的效果， 同时又阻止了象问题2中描述的那种转换
         typedef T* (TC_ScopedPtr::*unspecified_bool_type)() const;
        operator unspecified_bool_type() const
        {
            return m_p ? &TC_ScopedPtr::get : 0;
        }
```