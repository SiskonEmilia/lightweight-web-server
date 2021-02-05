#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <memory>


class ConnectionSocket {

    socklen_t mLen;
    sockaddr_in mAddr;
    int fd;

public:
    ConnectionSocket() : fd(-1) { };

    void setFd(const int fd) {
        this->fd = fd;
    }

    const int getFd() {
        return fd;
    }

    void close();

    ~ConnectionSocket();
};