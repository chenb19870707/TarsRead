# 智能指针类 tc_autoptr


### code：
[tc_autoptr.h](https://github.com/Tencent/Tars/blob/master/cpp/util/include/util/tc_autoptr.h)   

## 知识点

### 与std::autoptr实现不同

拷贝时原指针计数-1，新指针计数+1,r并没有失效
```
    template<typename Y>
    TC_AutoPtr& operator=(const TC_AutoPtr<Y>& r)
    {
        if(_ptr != r._ptr)
        {
            if(r._ptr)
            {
                r._ptr->incRef();
            }

            T* ptr = _ptr;
            _ptr = r._ptr;

            if(ptr)
            {
                ptr->decRef();
            }
        }
        return *this;
    }
```
析构是decRef计数为0调用的

```
	A *a = new A("a");
	A *b = new A("b");
	cout << a << endl;
	cout << b << endl;

	cout << "TC_AutoPtr release begin" << endl;
	{
		TC_AutoPtr<A> spa(a);
		TC_AutoPtr<A> spb(b);

		spb = spa;

		cout << spa->getRef()  <<endl;
		cout << spb->getRef() << endl;

		cout << spa->test()  <<endl;
		cout << spb->test() << endl;
	}
	cout << "TC_AutoPtr release end" << endl;

	cout << a->test() << endl;
	cout << b->test() << endl;
```

结果：
```
0x1f8d040
0x1f8d090
TC_AutoPtr release begin
release:b
decRef(),curRef0
2
2
a
a
decRef(),curRef1
release:a
decRef(),curRef0
TC_AutoPtr release end
段错误(吐核)



```


而std::autoptr =运算符如果不是操作同一个对象的时候会先删除左操作数指向的对象，后面指向右操作数指向的对象，再release()右操作数，从前面可以看到release() 只是将指针置空，并没有释放内存。从这里可以看到永远只有一个auto_ptr对象指向它。
```
  _Tp* release() __STL_NOTHROW {
    _Tp* __tmp = _M_ptr;
    _M_ptr = 0;
    return __tmp;
  }

  auto_ptr& operator=(auto_ptr& __a) __STL_NOTHROW {
    if (&__a != this) {
      delete _M_ptr;
      _M_ptr = __a.release();
    }
    return *this;
  }

```
测试程序：
```
class A: public TC_HandleBaseT<int>
{
	public:
	A(const string& name) { _name = name;}
	string test() { return _name; }

	~A() { cout << "release:" << _name << endl;}

private:
	string _name;
};

int main()
{
	A *a = new A("a");
	A *b = new A("b");
	cout << a << endl;
	cout << b << endl;
	
	cout << "autoptr release begin" << endl;
	{
		std::auto_ptr<A> apa(a);
		std::auto_ptr<A> apb(b);

		apb = apa;

		cout << apa.get()  <<endl;
		cout << apb.get() << endl;
	}
	cout << "autoptr release end" << endl;

	cout << a->test() << endl;
	cout << b->test() << endl;
```

结果
```
0x900040
0x900090
autoptr release begin
release:b
0
0x900040
release:a
autoptr release end
段错误(吐核)

```

### 空指针异常类TC_AutoPtrNull_Exception

```
/**
* @brief 空指针异常
*/
struct TC_AutoPtrNull_Exception : public TC_Exception
{
    TC_AutoPtrNull_Exception(const string &buffer) : TC_Exception(buffer){};
    ~TC_AutoPtrNull_Exception() throw(){};
};
```

### 智能指针基类TC_HandleBaseT

所有需要智能指针支持的类都需要从该对象继承   
内部采用引用计数TC_Atomic实现，对象可以放在容器中

#### 构造函数、拷贝构造、析构函数
```
protected:
    TC_HandleBaseT() : _atomic(0), _bNoDelete(false)
    {
    }


    TC_HandleBaseT(const TC_HandleBaseT&) : _atomic(0), _bNoDelete(false)
    {
    }


    virtual ~TC_HandleBaseT()
    {
    }
```
都是protected,禁止TC_HandleBaseT拷贝和new

#### operate= 函数

标准写法是
```
MyClass& MyClass::operator=(const MyClass &rhs)
{  
    //是同一个对象，直接返回*this
    if (this == &rhs)   
      return *this;      
  
    //否则不是一个对象 Deallocate, allocate new space, copy values...  
    ...
    return *this;  
  }
```
**这里没有 判断是否相等就直接返回*this是因为????*

```
TC_HandleBaseT& operator=(const TC_HandleBaseT&)
{
    return *this;
}
```

#### 增加计数

```
 void incRef() { _atomic.inc_fast(); }
```

#### 减少计数, 当计数==0时, 且需要删除数据时, 释放对象

```
    void decRef()
    {
        if(_atomic.dec_and_test() && !_bNoDelete)
        {
            _bNoDelete = true;
            delete this;
        }
    }
```

#### 获取计数
```
 int getRef() const        { return _atomic.get(); }
```


###  TC_HandleBaseT的int特化实现
```
template<>
inline void TC_HandleBaseT<int>::incRef() 
{ 
    //__sync_fetch_and_add(&_atomic,1);
    ++_atomic; 
}

template<> 
inline void TC_HandleBaseT<int>::decRef()
{
    //int c = __sync_fetch_and_sub(&_atomic, 1);
    //if(c == 1 && !_bNoDelete)
    if(--_atomic == 0 && !_bNoDelete)
    {
        _bNoDelete = true;
        delete this;
    }
}

template<> 
inline int TC_HandleBaseT<int>::getRef() const        
{ 
    //return __sync_fetch_and_sub(const_cast<volatile int*>(&_atomic), 0);
    return _atomic; 
} 
```

###  TC_HandleBaseT的TC_Atomic特化实现

```
typedef TC_HandleBaseT<TC_Atomic> TC_HandleBase;
```

###  智能指针类TC_AutoPtr

#### 构造函数

1.  用原生指针初始化, 计数+1
```
    TC_AutoPtr(T* p = 0)
    {
        _ptr = p;

        if(_ptr)
        {
            _ptr->incRef();
        }
    }
```
2.用其他智能指针r的原生指针初始化, 计数+1
```
template<typename Y>
    TC_AutoPtr(const TC_AutoPtr<Y>& r)
    {
        _ptr = r._ptr;

        if(_ptr)
        {
            _ptr->incRef();
        }
    }
```
3.拷贝构造, 计数+1
```
    TC_AutoPtr(const TC_AutoPtr& r)
    {
        _ptr = r._ptr;

        if(_ptr)
        {
            _ptr->incRef();
        }
    }
```
4.析构函数，计数-1
```
    ~TC_AutoPtr()
    {
        if(_ptr)
        {
            _ptr->decRef();
        }
    }
```

#### 赋值, 普通指针 operator= 

p引用计数+1 ,this所指对象引用计数-1

此处T 必须是继承TC_HandleBaseT的，所以有incRef()方法
```
    TC_AutoPtr& operator=(T* p)
    {
        if(_ptr != p)
        {
            if(p)
            {
                p->incRef();
            }

            T* ptr = _ptr;
            _ptr = p;

            if(ptr)
            {
                ptr->decRef();
            }
        }
        return *this;
    }
```

#### 赋值, 其他类型智能指针 operator= 

p引用计数+1 ,this所指对象引用计数-1

```
    template<typename Y>
    TC_AutoPtr& operator=(const TC_AutoPtr<Y>& r)
    {
        if(_ptr != r._ptr)
        {
            if(r._ptr)
            {
                r._ptr->incRef();
            }

            T* ptr = _ptr;
            _ptr = r._ptr;

            if(ptr)
            {
                ptr->decRef();
            }
        }
        return *this;
    }
```

#### 赋值, 该类型其他执政指针 operator=

```
    TC_AutoPtr& operator=(const TC_AutoPtr& r)
    {
        if(_ptr != r._ptr)
        {
            if(r._ptr)
            {
                r._ptr->incRef();
            }

            T* ptr = _ptr;
            _ptr = r._ptr;

            if(ptr)
            {
                ptr->decRef();
            }
        }
        return *this;
    }
```

####  将其他类型的智能指针换成当前类型的智能指针
```
    template<class Y>
    static TC_AutoPtr dynamicCast(const TC_AutoPtr<Y>& r)
    {
        return TC_AutoPtr(dynamic_cast<T*>(r._ptr));
    }
```

#### 将其他原生类型的指针转换成当前类型的智能指针
```
    template<class Y>
    static TC_AutoPtr dynamicCast(Y* p)
    {
        return TC_AutoPtr(dynamic_cast<T*>(p));
    }
```

#### 获取原生指针 get()
```
    T* get() const
    {
        return _ptr;
    }
```

#### 调用 operator->() 
```
    T* operator->() const
    {
        if(!_ptr)
        {
            throwNullHandleException();
        }

        return _ptr;
    }
```

#### 引用 operator*()
```
    T& operator*() const
    {
        if(!_ptr)
        {
            throwNullHandleException();
        }

        return *_ptr;
    }
```

#### 是否有效     operator bool()
```
    operator bool() const
    {
        return _ptr ? true : false;
    }
```


#### 交换指针 swap(TC_AutoPtr& other)
```
    void swap(TC_AutoPtr& other)
    {
        std::swap(_ptr, other._ptr);
    }
```

#### ==判断 operator==(const TC_AutoPtr<T>& lhs, const TC_AutoPtr<U>& rhs)

智能指针指向同一个对象即为真

```
template<typename T, typename U>
inline bool operator==(const TC_AutoPtr<T>& lhs, const TC_AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l == *r;
    }
    else
    {
        return !l && !r;
    }
}
```

#### 不等于判断  operator!=(const TC_AutoPtr<T>& lhs, const TC_AutoPtr<U>& rhs)

同为空或者指向同一个对象为相等
```
template<typename T, typename U>
inline bool operator!=(const TC_AutoPtr<T>& lhs, const TC_AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l != *r;
    }
    else
    {
        return l || r;
    }
}
```

#### 小于判断, 用于放在map等容器中 operator<(const TC_AutoPtr<T>& lhs, const TC_AutoPtr<U>& rhs)

```
template<typename T, typename U>
inline bool operator<(const TC_AutoPtr<T>& lhs, const TC_AutoPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l < *r;
    }
    else
    {
        return !l && r;
    }
```