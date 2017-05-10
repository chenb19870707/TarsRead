# 异常类类tc_atomic


### code：
[tc_atomic.h](https://github.com/Tencent/Tars/blob/master/cpp/util/include/util/tc_ex.h)   
[tc_atomic.cpp](https://github.com/Tencent/Tars/blob/master/cpp/util/src/tc_ex.cpp)

## 知识点

获取堆栈
```
void TC_Exception::getBacktrace()
{
    void * array[64];
    int nSize = backtrace(array, 64);
    char ** symbols = backtrace_symbols(array, nSize);

    for (int i = 0; i < nSize; i++)
    {
        _buffer += symbols[i];
        _buffer += "\n";
    }
    free(symbols);
}
```