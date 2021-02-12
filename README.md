# Lightweight WEB Server

本项目旨在构建一个轻量级多线程 C++ HTTP 服务器。

关键字：Epoll、RAII锁、Reactor

## 简介

### 版本日志

#### version 0.0.0 技术验证 

- **开发目标**
  
    实现一个打印请求文本的空白服务器。验证 Epoll + Reactor 的技术可行性。

- **新增特性**
  - 基于 Epoll 的 I/O 多路复用
  - 基于线程池的并发处理
  - 提供 RAII 风格的互斥锁
  - Reactor 并发模型
  - HTTP GET 请求解析

## Commit 命名方法

格式：

```shell
[${TAG}] ${VERSION}: ${CONTENT} 
```

TAG 列表：

- [Feat] 新特性
- [Fix] 修复问题
- [Refactor] 重构代码
- [Style] 风格整改
- [Docs] 文档修改
- [Others] 其他修改