# Lightweight WEB Server

*v0.1.0: Venti. Wish your lightness.*

![banner](./assets/banner010.jpg)

**轻量、易用、高效、支持自定义服务** 且 **易读** 的 Linux 多线程 HTTP 服务器。基于 epoll 构建的 I/O 多路复用并发模型，支持长连接、大文件传输，提供自带的 **静态文件服务器** 接口。目前支持 GET 请求规则自定义。

## 简介

### 使用指南

#### 环境要求

本项目在如下环境中开发并通过测试：

- OS: Ubuntu 16.04
- Compiler：g++ 5
- cmake 3.19.4

本项目要求您至少满足以下条件：

- OS：Linux，内核版本 >= 2.6.17
- Compiler：支持 C++ 11
- cmake 3.5+

#### 编译方法

如果您想直接编译 **示例程序**，您可以遵循以下方法：

**编译前：**

1. 在 `CMakeList.txt` 中将 `g++5` 修改为您的环境所支持的编译器链接名
2. 在工作目录下创建 `build` 目录

**编译命令：**

```shell
cmake -S . -B build
make -C build
```

完成后您可以在 `build` 目录下找到可执行文件 `lserver`。

#### 如何使用本库创建服务器

如果您想直接使用本库附带的静态文件服务，您可以：

```cpp
// 包含必要头文件
#include "http/HttpServer.h" // 服务器核心头文件
#include "middleware/StaticFile.h" // 静态服务头文件

// 创建服务器
HttpServer server;
// 创建静态文件服务，指定您需要暴露的静态文件地址和挂载的服务器地址
StaticFile filer_service("path/to/static/folder", "mount/point");
// 绑定服务器和文件服务
server.Get(file_service.getRegex(), file_service.getCallback());
// 启动服务器
server.run();
```

或者，您也可以定制服务器功能：

```cpp
server.Get(your_regex, your_callback);
```

此处的 `your_callback` 为回调函数，当服务器收到与 `your_regex` 正则式匹配的 `GET` 请求时，会为此函数提供包含 `request` 和 `response` 的 `HttpData` 对象引用，用户可以根据请求自行修改响应中的缓存区或者文件地址，从而实现定制服务器的服务规则。


### 版本日志

#### version 0.1.0: Venti 静态网页服务器

*Wish your lightness.*

![banner](./assets/banner010.jpg)

- **开发目标**

    实现一个可以托管文件目录的静态服务器

- **更新内容**

  - HTTP 格式的响应报文
  - 可以托管特定的文件目录，使之暴露到服务器的相应端口上
  - 实现大文件的基于 epoll 的异步传输
  - 验证通过静态服务器测试（Chrome、Safari、Edge、curl）

#### version 0.0.0: ATRI 技术验证 

*Learnt to be...*

![banner](./assets/banner011.jpg)

- **开发目标**
  
    实现一个打印请求文本的空白服务器。验证 Epoll + Reactor 的技术可行性。

- **新增特性**
  - 基于 Epoll 的 I/O 多路复用
  - 基于线程池的并发处理
  - 提供 RAII 风格的互斥锁
  - Reactor 并发模型
  - HTTP GET 请求解析

### 项目结构



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