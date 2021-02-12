#include <cstring>
#include <iostream>

#include "sockets/ListenSocket.h"
#include "sockets/ConnectionSocket.h"

using std::cout;
using std::endl;

void ListenSocket::bind() {
    int ret = ::bind(getSocketFd(), (struct sockaddr*)&served_addr, sizeof(served_addr));
    if (ret == -1) {
        cout << "ListenSocket: Failed to bind port in file <" << __FILE__ << "> "
             << "at " << __LINE__ << endl;
        exit(-1);
    }
}

/**
 * @brief 封装用于启动 listen socket 监听的行为
*/
void ListenSocket::listen() {
    int ret = ::listen(getSocketFd(), 4096);
    if (ret == -1) {
        cout << "ListenSocket: Failed to start listening in file <"
             << __FILE__ << "> "<< "at " << __LINE__ << endl;
        exit(-1);
    }
}

/**
 * @brief 按给定的 IP 地址和 端口号 初始化一个 listen socket
*/
ListenSocket::ListenSocket(int port, const char *ip) {
    // 清空地址结构体
    bzero(&served_addr, sizeof(served_addr));
    // 指定协议簇：TCP/IP
    served_addr.sin_family = AF_INET;
    // 将 port 转化为大端形式
    served_addr.sin_port = htons(port);
    // 设置 IP 地址
    if (ip != nullptr) {
        inet_pton(AF_INET, ip, &served_addr.sin_addr);
    } else {
        served_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd >= 0) {
        setSocketFd(socket_fd);
    } else {
        cout << "ListenSocket: Failed to apply for socket in file <"
             << __FILE__ << "> "<< "at " << __LINE__  << endl;
        exit(-1);
    }

    setNonBlocking();
    setReuseable();
}

/**
 * @brief 尝试接受一个客户端发起的链接请求，若成功则为 connection socket 引用设置相关域
*/
int ListenSocket::accept(ConnectionSocket &connection) const {
    int connection_fd = ::accept(getSocketFd(), nullptr, nullptr);
    if (connection_fd < 0) {
        cout << "ListenSocket: Failed to accept connection" << endl;
    } else {
        connection.setSocketFd(connection_fd);
    }
    return connection_fd;
}