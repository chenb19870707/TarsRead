# 智能指针类 TC_ScopedPtr

## 知识点

### 1.TC_FunctionTraits 

返回类型的traits，从0个参数到9个参数
```
       template <typename Signature>
        struct TC_FunctionTraits;

        template <typename R>
        struct TC_FunctionTraits<R()> {
            typedef R ReturnType;
        };

        template <typename R, typename A1>
        struct TC_FunctionTraits<R(A1)> {
            typedef R ReturnType;
            typedef A1 A1Type;
        };

        template <typename R, typename A1, typename A2>
        struct TC_FunctionTraits<R(A1, A2)> {
            typedef R ReturnType;
            typedef A1 A1Type;
            typedef A2 A2Type;
        };

        template <typename R, typename A1, typename A2, typename A3>
        struct TC_FunctionTraits<R(A1, A2, A3)> {
            typedef R ReturnType;
            typedef A1 A1Type;
            typedef A2 A2Type;
            typedef A3 A3Type;
        };

        template <typename R, typename A1, typename A2, typename A3, typename A4>
        struct TC_FunctionTraits<R(A1, A2, A3, A4)> {
            typedef R ReturnType;
            typedef A1 A1Type;
            typedef A2 A2Type;
            typedef A3 A3Type;
            typedef A4 A4Type;
        };

        template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
        struct TC_FunctionTraits<R(A1, A2, A3, A4, A5)> {
            typedef R ReturnType;
            typedef A1 A1Type;
            typedef A2 A2Type;
            typedef A3 A3Type;
            typedef A4 A4Type;
            typedef A5 A5Type;
        };

        template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
        struct TC_FunctionTraits<R(A1, A2, A3, A4, A5, A6)> {
            typedef R ReturnType;
            typedef A1 A1Type;
            typedef A2 A2Type;
            typedef A3 A3Type;
            typedef A4 A4Type;
            typedef A5 A5Type;
            typedef A6 A6Type;
        };

        template <typename R, typename A1, typename A2, typename A3, typename A4,
                  typename A5, typename A6, typename A7>
        struct TC_FunctionTraits<R(A1, A2, A3, A4, A5, A6, A7)> {
            typedef R ReturnType;
            typedef A1 A1Type;
            typedef A2 A2Type;
            typedef A3 A3Type;
            typedef A4 A4Type;
            typedef A5 A5Type;
            typedef A6 A6Type;
            typedef A7 A7Type;
        };

        template <typename R, typename A1, typename A2, typename A3, typename A4,
                  typename A5, typename A6, typename A7, typename A8>
        struct TC_FunctionTraits<R(A1, A2, A3, A4, A5, A6, A7, A8)> {
            typedef R ReturnType;
            typedef A1 A1Type;
            typedef A2 A2Type;
            typedef A3 A3Type;
            typedef A4 A4Type;
            typedef A5 A5Type;
            typedef A6 A6Type;
            typedef A7 A7Type;
            typedef A8 A8Type;
        };

        template <typename R, typename A1, typename A2, typename A3, typename A4,
                  typename A5, typename A6, typename A7, typename A8, typename A9>
        struct TC_FunctionTraits<R(A1, A2, A3, A4, A5, A6, A7, A8, A9)> {
            typedef R ReturnType;
            typedef A1 A1Type;
            typedef A2 A2Type;
            typedef A3 A3Type;
            typedef A4 A4Type;
            typedef A5 A5Type;
            typedef A6 A6Type;
            typedef A7 A7Type;
            typedef A8 A8Type;
            typedef A9 A9Type;
        };
```

### 2.函数适配器，从0个参数到9个参数，重写operator()
每个参数有两种重载
1. 返回R类型，有N个参数的普通函数。   
2. 返回R类型，有N个参数的类成员函数。
3. 返回R类型，有N个参数的类const成员函数。

```
        template <typename Signature>
        class TC_FunctorAdapter;

        // Function: arity 0.普通函数
        template <typename R>
        class TC_FunctorAdapter<R (*)()> {
        public:
            typedef R (RunType)();
            
            explicit TC_FunctorAdapter(R (*f)())
            : m_pf(f)
            { }

            R operator()()
            { return (*m_pf)(); }
    
        private:
            R (*m_pf)();
        };

        // Method: arity 0.类成员函数
        template <typename R, typename T>
        class TC_FunctorAdapter<R (T::*)()> {
        public:
            typedef R (RunType)(T*);
            
            explicit TC_FunctorAdapter(R (T::*pmf)())
            : m_pmf(pmf)
            { }

            R operator()(T *pobj)
            { return (pobj->*m_pmf)(); }
        
        private:
            R (T::*m_pmf)();
        };

        // Const method: arity 0.
        template <typename R, typename T>
        class TC_FunctorAdapter<R (T::*)() const> {
        public:
            typedef R (RunType)(const T*);
            
            explicit TC_FunctorAdapter(R (T::*pmf)() const)
            : m_pmf(pmf)
            { }

            R operator()(const T *pobj)
            { return (pobj->*m_pmf)(); }
        
        private:
            R (T::*m_pmf)() const;
        };
        //省略1~9的实现
```

### 3.参数包装器：用于对bind绑定的参数类型 的内存管理

1. TC_UnretainedWrapper调用者负责管理参数的内存，即TC_UnretainedWrapper不销毁内存
```
        template <typename T>
        class TC_UnretainedWrapper {
        public:
            explicit TC_UnretainedWrapper(T *p) : m_ptr(p) { }
            T *get() const { return m_ptr; }
        private:
            T *m_ptr;
        };
```

2. TC_OwnedWrapper 回调函数负责管理绑定参数的内存，即TC_OwnedWrapper析构函数负责回收内存
```
      template <typename T>
        class TC_OwnedWrapper {
        public:
            explicit TC_OwnedWrapper(T *p) : m_ptr(p) { }
            ~TC_OwnedWrapper() 
			{ 
				if(m_ptr != NULL)
				{
					delete m_ptr; 
					m_ptr = NULL;
				}
			}
            TC_OwnedWrapper(const TC_OwnedWrapper& o)
            {
                m_ptr = o.m_ptr;
                o.m_ptr = NULL;
            }
            TC_OwnedWrapper& operator=(const TC_OwnedWrapper& o)
            {
				if(m_ptr != NULL)
				{
					delete m_ptr;
					m_ptr = NULL;
				}
                m_ptr = o.m_ptr;
                o.m_ptr = NULL;
            }
            T *get() const { return m_ptr; }
            
        private:
            mutable T *m_ptr;
        };
```

3. TC_SharedWrapper 指对象被调用者和回调函数共享，引用计数为0的时候销毁

```
       template <typename T>
        struct TC_SharedWrapper {
        public:
            explicit TC_SharedWrapper(const TC_SharedPtr<T>& p)
                : m_ptr(p)
            { }
            T *get() const { return m_ptr.get(); }
        private:
            TC_SharedPtr<T> m_ptr;
        };
        
```

### 4.参数解包器，针对3中的打包器

```
        template <typename T>
        struct TC_UnwrapTraits {
            typedef const T& ForwardType;
            static ForwardType unwrap(const T& o) { return o; }
        };
        
        template <typename T>
        struct TC_UnwrapTraits<TC_UnretainedWrapper<T> > {
            typedef T* ForwardType;
            static ForwardType unwrap(const TC_UnretainedWrapper<T>& unretained)
            { return unretained.get(); }
        };

        template <typename T>
        struct TC_UnwrapTraits<TC_OwnedWrapper<T> > {
            typedef T* ForwardType;
            static ForwardType unwrap(const TC_OwnedWrapper<T>& owned)
            { return owned.get(); }
        };

        template <typename T>
        struct TC_UnwrapTraits<TC_SharedWrapper<T> > {
            typedef T* ForwardType;
            static ForwardType unwrap(const TC_SharedWrapper<T>& shared)
            { return shared.get(); }
        };
```


### 5.TC_BindState 存储所有的传递给bind的参数，从0个参数到9个参数
```
    template <typename FunctorType, typename RunType, typename BoundArgsType>
        struct TC_BindState;
        
        template <typename FunctorType, typename RunType>
        class TC_BindState<FunctorType, RunType, void()> : public TC_BindStateBase {
        public:
            typedef TC_Invoker<TC_BindState, RunType, 0> InvokerType;
            typedef typename InvokerType::UnboundRunType UnboundRunType;
            
            explicit TC_BindState(const FunctorType& functor)
                : m_functor(functor)
            { }

            FunctorType m_functor;
        };

        template <typename FunctorType, typename RunType, typename A1>
        class TC_BindState<FunctorType, RunType, void(A1)> : public TC_BindStateBase {
        public:
            typedef TC_Invoker<TC_BindState, RunType, 1> InvokerType;
            typedef typename InvokerType::UnboundRunType UnboundRunType;
            typedef TC_UnwrapTraits<A1> Bound1UnwrapTraits;
            
            TC_BindState(const FunctorType& functor, const A1& a1)
                : m_functor(functor),
                  m_a1(a1)
            { }

            FunctorType m_functor;
            A1 m_a1;
        };

        //省略2~9
``` 

### 6.TC_Invoker 从TC_BindState解析参数，并且执行callbak

没有参数直接调用
```
        template <typename TC_BindState, typename RunType, int NumBound>
        class TC_Invoker;

        // Arity: 0 -> 0
        template <typename TC_BindState, typename R>
        class TC_Invoker<TC_BindState, R(), 0> {
        public:
            typedef R (UnboundRunType)();
            
            static R invoke(TC_BindStateBase *base)
            {
                TC_BindState *bind_state = static_cast<TC_BindState*>(base);
                return (bind_state->m_functor)();
            }
        };
```

1 个参数，如果带了一个参数则直接调用，否则从TC_BindState中解析出一个参数调用
```
       // Arity: 1 -> 1
        template <typename TC_BindState, typename R, typename A1>
        class TC_Invoker<TC_BindState, R(A1), 0> {
        public:

            typedef R (UnboundRunType)(A1);
            
            static R invoke(TC_BindStateBase *base,
                            typename TC_CallbackParamTraits<A1>::ForwardType a1)
            {
                TC_BindState *bind_state = static_cast<TC_BindState*>(base);
                return (bind_state->m_functor)(a1);
            }
        };

        // Arity: 1 -> 0
        template <typename TC_BindState, typename R, typename A1>
        class TC_Invoker<TC_BindState, R(A1), 1> {
        public:

            typedef R (UnboundRunType)();
            
            static R invoke(TC_BindStateBase *base)
            {
                TC_BindState *bind_state = static_cast<TC_BindState*>(base);

                typedef typename TC_BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
                typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
                return (bind_state->m_functor)(a1);
            }
        };
        //省略其它
```