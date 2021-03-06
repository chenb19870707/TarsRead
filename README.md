# Tars源码阅读

@(Tars)[C++| 腾讯]

**Tars**是腾讯从2008年到今天一直在使用的后台逻辑层的统一应用框架TAF（Total Application Framework），目前支持C++和Java两种语言。该框架为用户提供了涉及到开发、运维、以及测试的一整套解决方案，帮助一个产品或者服务快速开发、部署、测试、上线。 它集可扩展协议编解码、高性能RPC通信框架、名字路由与发现、发布监控、日志统计、配置管理等于一体，通过它可以快速用微服务的方式构建自己的稳定可靠的分布式应用，并实现完整有效的服务治理。

目前该框架在腾讯内部，各大核心业务都在使用，颇受欢迎，基于该框架部署运行的服务节点规模达到上万个。。

Tars完成度非常高，具备了服务器框架的所有功能，非常值得研究与学习：
 
- **负载均衡** ：
- **容错保护** ：
- **过载保护** ：
- **消息染色** ：
- **IDC分组** ：
- **数据监控** ：
- **集中配置** ：

-------------------

[TOC]

## 工具篇
Tars基础工具代码位于cpp/util。

###  原子操作类tc_atomic
[tc_atomic](util/tc_atomic/tc_atomic.md)

###  异常类  tc_ex
[tc_ex](util/tc_ex.md)

###  智能指针类  tc_autoptr
[tc_autoptr](util/tc_autoptr/tc_autoptr.md)

###  共享智能指针类  tc_sharedptr
[tc_sharedptr](util/tc_sharedptr/tc_sharedptr.md)

###  智能指针类  tc_scopeptr
[tc_scopeptr](util/tc_scopeptr.md)


### Base64编解码类 tc_base64
[tc_base64](util/tc_base64.md)