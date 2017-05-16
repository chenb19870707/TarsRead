# 共享智能指针类 tc_sharedptr

## 知识点

### 1.C++ Complete Type

typedef char type_must_be_complete[sizeof(T) ? 1 : -1] 
确保T是一个完整类型；
在C++中，类型有Complete type和Incomplete type之分，对于Complete type, 它的大小在编译时是可以确定的，而对于Incomplete type, 它的大小在编译时是不能确定的。所以，上面的代码中，如果C是Incomplete type的话，sizeof就会在编译时报错，从而达到了我们检查C是否是Complete type的目的。

```
namespace tars 
{
    template <typename T>
    inline void tc_checked_delete(T *p)
    {
        typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        delete p;
    }

    template <typename T>
    inline void tc_checked_array_delete(T *p)
    {
        typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        delete[] p;
    }

}
```

### 2.引用计数基类 tc_shared_count_base

1. 拷贝构造和operator=都是私有，不能拷贝构造和赋值操作
2.  dispose() is called when m_use_count drops to zero, to release resources managed by this object.
```
       class tc_shared_count_base 
        {
        public:

            tc_shared_count_base()
            : m_use_count(1)
            {}

            virtual ~tc_shared_count_base()
            {}
            
            int use_count() const
            {
                return m_use_count.get();
            }

            void increment()
            {
                ++m_use_count;
            }

            void release()
            {
                if (m_use_count.dec_and_test()) 
                {
                    dispose();
                    delete this;
                }
            }

            virtual void dispose() = 0;
            
        private:  
            tc_shared_count_base(const tc_shared_count_base&);
            void operator=(const tc_shared_count_base&);
            
            tars::TC_Atomic m_use_count;
        };

    }
}
```

### 2.引用计数基实现类 tc_shared_count_base

提供了dispose方法，删除m_px所指的T对象

```
        template <typename T>
        class tc_shared_count_impl_p : public tc_shared_count_base 
        {
        public:

            template <typename U>
            tc_shared_count_impl_p(U *p)
            : m_px(p)
            {}

            virtual void dispose()
            {
                tc_checked_delete(m_px);
            }
            
        private:

            T *m_px;
        };
```

### 3.特化模板类型定义

#### 内部常量

* true_type  
* false_type
* tc_is_void <T>
* tc_is_void<void>
* tc_is_non_const_reference 
* tc_is_non_const_reference<T&>
* tc_is_non_const_reference<const T&>
* tc_is_raw_pointer
* tc_is_raw_pointer<T*>


```
    template <typename T, T v>
    struct tc_integral_constant 
	{
        typedef T value_type;
        typedef tc_integral_constant<T, v> type;
        static const T value = v;
    };

    template <typename T, T v> const T tc_integral_constant<T, v>::value;
    
    typedef tc_integral_constant<bool, true> true_type;
    typedef tc_integral_constant<bool, false> false_type;

    template <typename T> struct tc_is_void : false_type {};
    template <> struct tc_is_void<void> : true_type {};
    
    template <typename T> struct tc_is_non_const_reference : false_type {};
    template <typename T> struct tc_is_non_const_reference<T&> : true_type {};
    template <typename T> struct tc_is_non_const_reference<const T&> : false_type {};

    template <typename T> struct tc_is_raw_pointer : false_type {};
    template <typename T> struct tc_is_raw_pointer<T*> : true_type {};

	// Check if two types are the same
    template <typename U, typename V>
    struct tc_is_same_type : tc_integral_constant<bool, false> {};

    template <typename U>
    struct tc_is_same_type<U, U> : tc_integral_constant<bool, true> {};
```

### 4. TC_EnableSharedFromThis

#### 为什么需要EnableSharedFromThis
```
struct A {
  void func() {
    // only have "this" ptr ?
  }
};

int main() {
  A* a;
  std::shared_ptr<A> sp_a(a);
}
```


这里就需要用enable_shared_from_this改写:

当A* a被shared_ptr托管的时候,如何在func获取自身的shared_ptr成了问题.
如果写成:

```
void func() {
  std::shared_ptr<A> local_sp_a(this);
  // do something with local_sp_a
}
```

又用a新生成了一个shared_ptr: local_sp_a, 这个**在生命周期结束的时候可能将a直接释放掉**,所以需要enablefromthis。

```
struct A : public enable_shared_from_this {
  void func() {
    std::shared_ptr<A> local_sp_a = shared_from_this();
    // do something with local_sp
  }
};
```
shared_from_this会从weak_ptr安全的生成一个自身的shared_ptr.



####  成员变量：
```
        mutable T *m_this;                                         //指针
        mutable detail::tc_shared_count_base *m_owner_use_count;  //引用计数
```

####  构造函数、拷贝构造函数、析构函数、operator=是protected（内部类）：

```
    protected:
        TC_EnableSharedFromThis()
        : m_this(NULL)
        , m_owner_use_count(NULL)
        { }

        TC_EnableSharedFromThis(const TC_EnableSharedFromThis&)
        { }

        TC_EnableSharedFromThis& operator=(const TC_EnableSharedFromThis&)
        {
            return *this;
        }

        ~TC_EnableSharedFromThis()
        { }
```

####  sharedFromThis
```
        TC_SharedPtr<T> sharedFromThis()
        {
            TC_SharedPtr<T> p(m_this, m_owner_use_count);
            return p;
        }

        /**
         * Same as above.
         */
        TC_SharedPtr<const T> sharedFromThis() const
        {
            TC_SharedPtr<const T> p(m_this, m_owner_use_count);
            return p;
        }
```

在TC_SharedPtr声明了友元函数:
```
        template <typename U> friend class TC_SharedPtr;
        template <typename U> friend class TC_EnableSharedFromThis;
```

故而上面调用的是TC_SharedPtr的私有构造函数
```
        // TC_EnableSharedFromThis support
        template <typename U>
        explicit TC_SharedPtr(U *p, detail::tc_shared_count_base *pn)
        : m_px(p), m_pn(pn)
        {
            if (m_pn == NULL)
                TC_SharedPtr(p);
            else
                m_pn->increment();
        }
```

如果引用计数为0，则构造sharedptr，否则，直接增加引用计数

！！！这样就防止了上面提到的析构问题。

### 4. 共享智能指针TC_SharedPtr

####  成员变量：
```
        T *m_px;                             //指针
        detail::tc_shared_count_base *m_pn;  //引用计数
```

####  构造函数：
```
        template <typename U>
        explicit TC_SharedPtr(U *p)
        : m_px(p)
		, m_pn(NULL)
        {
            try 
			{
                if (m_px)
                    m_pn = new detail::tc_shared_count_impl_p<U>(p);
            } 
			catch (...) 
			{
                tc_checked_delete(p);
                throw;
            }
            detail::tc_sp_enable_shared_from_this(this, p);
        }
```