# Lightweight WEB Server

*v0.1.1: Zhongli. Solid as stones.*

![banner](./assets/banner011.jpg)

**轻量、易用、高效、支持自定义服务** 且 **易读** 的 Linux 多线程 HTTP 服务器。基于 epoll 构建的 I/O 多路复用并发模型，支持长连接、大文件传输、并发限制，提供自带的 **静态文件服务器** 接口。目前支持 GET 请求规则自定义。

- [Lightweight WEB Server](#lightweight-web-server)
  - [使用指南](#使用指南)
    - [环境要求](#环境要求)
    - [编译方法](#编译方法)
    - [如何使用本库创建服务器](#如何使用本库创建服务器)
  - [版本日志](#版本日志)
    - [version 0.1.1 Zhongli 压力测试](#version-011-zhongli-压力测试)
    - [version 0.1.0: Venti 静态网页服务器](#version-010-venti-静态网页服务器)
    - [version 0.0.0: ATRI 技术验证](#version-000-atri-技术验证)
  - [性能测试](#性能测试)
    - [测试环境](#测试环境)
    - [测试结果](#测试结果)
    - [结果分析](#结果分析)
  - [项目结构](#项目结构)
  - [Commit 命名方法](#commit-命名方法)

## 使用指南

### 环境要求

本项目在如下环境中开发并通过测试：

- OS: Ubuntu 16.04
- Compiler：g++ 5
- cmake 3.19.4

本项目要求您至少满足以下条件：

- OS：Linux，内核版本 >= 2.6.17
- Compiler：支持 C++ 11
- cmake 3.5+

### 编译方法

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

### 如何使用本库创建服务器

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


## 版本日志

### version 0.1.1 Zhongli 压力测试

*Solid as stones.*

![banner](./assets/banner011.jpg)

- **更新内容**
  - 新增了 **并发限制** 功能，自动获取当前最大打开文件描述符并限制并发，有效解决过大并发访问时导致的链接队列排满和文件描述符耗尽的问题。
  - 修复了 `HttpServer` 类中对于 `connection_map` 链接映射的竞争访问。
  - 修复了 `HttpTimer` 某些情况下无法正常释放链接的问题。
  - 修复了客户端崩溃时触发服务端未屏蔽的 `SIGPIPE` 信号，从而导致服务器异常停止运行的问题。
  - 添加了编译选项，用户可以统一设置部分日志的开关。
  - 优化了部分执行顺序，避免极端情况下的计时器竞争。
  - 针对 `valgrind` 内存检查结果进行了部分细节优化。
  - 通过了 [WebBench](https://github.com/EZLippi/WebBench) 压力测试，详情见 [性能测试板块](#性能测试)。

### version 0.1.0: Venti 静态网页服务器

*Wish your lightness.*

![banner](./assets/banner010.jpg)

- **开发目标**

    实现一个可以托管文件目录的静态服务器

- **更新内容**

  - HTTP 格式的响应报文
  - 可以托管特定的文件目录，使之暴露到服务器的相应端口上
  - 实现大文件的基于 epoll 的异步传输
  - 验证通过静态服务器测试（Chrome、Safari、Edge、curl）

### version 0.0.0: ATRI 技术验证 

*Learnt to be...*

![banner](./assets/banner000.jpg)

- **开发目标**
  
    实现一个打印请求文本的空白服务器。验证 Epoll + Reactor 的技术可行性。

- **新增特性**
  - 基于 Epoll 的 I/O 多路复用
  - 基于线程池的并发处理
  - 提供 RAII 风格的互斥锁
  - Reactor 并发模型
  - HTTP GET 请求解析

## 性能测试

本项目使用 Ubuntu 虚拟机进行仿真测试。使用 [WebBench](https://github.com/EZLippi/WebBench) 分别模拟 64/1024/10000 并发对服务器上的 `README.md`（107 Bytes） 进行访问。受限于测试设备，我们只能在同一台机器上同时模拟客户端和服务端，这无可避免的对测试造成了影响。

### 测试环境

宿主机配置：

- CPU：Intel i7-4710HQ@2.5ghz
- HDD：1TB 5400rpm HDD
- RAM：DDR3 2*8GB@1600MHz

虚拟机配置：

- OS：Ubuntu 16.04
- HDD：20GB
- RAM：8GB

### 测试结果

- 40000 最大文件描述符 64 并发访问 QPS：13264
- 40000 最大文件描述符 1024 并发访问 QPS：13295
- 40000 最大文件描述符 10240 并发访问 QPS：13221
- 1024 最大文件描述符 10240 并发访问 QPS：7149（此处结果些许失真）

### 结果分析

本项目在多种并发程度下都可以实现良好的服务效率，在文件描述符数量充足时，本项目可以完美应对 C10K 之类的高并发场景，在文件描述符不足时，本项目也可以通过并发控制避免耗尽文件描述符或是链接队列排满，从而导致无法继续提供服务。

## 项目结构

```txt
lightweight-web-server
|- assets
|- server         # 项目代码
||- include       # 头文件目录
|||- config       # 编译选项宏定义
|||- epoll        # 封装 epoll 函数
|||- fs           # RAII 文件管理
|||- http         # http 相关类
|||- middleware   # 中间件，包含静态文件服务工具类
|||- sockets      # sockets 包装类
|||- thread       # 多线程相关类，包含线程池和互斥锁
|||- timer        # 计时器
|||- utils        # 工具类
||- src
||| *.cpp         # 各类的实现文件
| CMakeLists.txt  # CMake 编译管理文件
| LICENSE
| README.md      
| .gitignore
```

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