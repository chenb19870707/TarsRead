# 智能指针类 tc_autoptr


### code：
[tc_autoptr.h](https://github.com/Tencent/Tars/blob/master/cpp/util/include/util/tc_autoptr.h)   

## 知识点

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