#include <iostream>

#include "http/HttpServer.h"
#include "timer/HttpTimer.h"
#include "sockets/ConnectionSocket.h"

using std::cout;
using std::endl;


std::shared_ptr<HttpTimer> 
HttpServer::createTimer(int timeout, int socket_fd) {
    auto iter = connection_map.find(socket_fd);
    if (iter == connection_map.end()) {
        cout << "HttpServer: Trying to create timer for the connection not existed." << endl;
        return std::shared_ptr<HttpTimer>();
    }
    std::shared_ptr<HttpTimer> timer(new HttpTimer(timeout, iter->second));
    timer_manager.append(std::static_pointer_cast<Timer>(timer));
    return timer;
}

void HttpServer::handleConnection(int timeout) {
    std::shared_ptr<ConnectionSocket> connection(new ConnectionSocket(*this));
    while(listen_socket.accept(*connection) > 0) {
        connection->setNonBlocking();
        // TODO: binds http data
        int ret = epoll_manager->addFd(connection->getSocketFd(), EpollManager::Single_Time_Event);
        if (ret < 0) {
            // 添加事件失败，打印错误报告
            cout << "HttpServer: Failed to handle connection for socket fd: " 
                 << connection->getSocketFd() << endl;
        } else {
            connection->setTimer(timeout);
            connection_map[connection->getSocketFd()] = connection;
        }
        std::shared_ptr<ConnectionSocket> new_connection(new ConnectionSocket(*this));
        connection.swap(new_connection);
    }
}

void HttpServer::removeConnection(const int socket_fd) {
    cout << "ToDel: Trying to remove connection: " << socket_fd 
         << "in <" << __FILE__ << "> at Line " << __LINE__ << endl;
    auto iter = connection_map.find(socket_fd);
    if (iter == connection_map.end()) {
        cout << "HttpServer: Failed to find connection to remove. Socket_fd: " << socket_fd << endl;
    } else {
        iter->second->removeTimer();
        connection_map.erase(iter);
        epoll_manager->delFd(socket_fd);
    }
}

void HttpServer::handleRequest(std::shared_ptr<ConnectionSocket> connection) {
    // TODO: 注意 buffer 要预留 \0 的位置
    cout << "Handling Request: " << connection->getSocketFd() << endl;
    char buffer[buffer_size + 1];
    bzero(buffer, buffer_size + 1);

    HttpRequestParser::HTTP_CODE ret_code = HttpRequestParser::HTTP_CODE::No_Request;
    ssize_t recv_size = 0;
    auto &request = connection->http_data.request;
    while (true) {
        recv_size = recv(connection->getSocketFd(), buffer, buffer_size, 0);
        if (recv_size == -1) {
            if (errno == EAGAIN) {
                if (ret_code == HttpRequestParser::Get_Request) {
                    // TODO: send msg
                    ::send(connection->getSocketFd(), "test message", 12, 0);
                    // TODO: check keep-alive
                    bool keep_alive = false;
                    if (keep_alive) {
                        request.clear();
                        epoll_manager->modFd(connection->getSocketFd(), EpollManager::Single_Time_Event);
                        connection->setTimer(20000);
                    } else {
                        removeConnection(connection->getSocketFd());
                    }
                } else {
                    // Wait for following msg
                    cout << "TODEL: Loop here: <" << __FILE__ << "> at Line " << __LINE__ << endl;
                    epoll_manager->modFd(connection->getSocketFd(), EpollManager::Single_Time_Event);
                    connection->setTimer(3000);
                }
            } else if (errno == EWOULDBLOCK) {
                cout << "TODEL: Loop here: <" << __FILE__ << "> at Line " << __LINE__ << endl;
                epoll_manager->modFd(connection->getSocketFd(), EpollManager::Single_Time_Event);
                connection->setTimer(3000);
            } else {
                cout << "ERRNO: " << errno << endl; 
                cout << "TODEL: Loop here: <" << __FILE__ << "> at Line " << __LINE__ << endl;
                removeConnection(connection->getSocketFd());
            }
            break;
        } else if (recv_size == 0) {
            cout << "HttpServer: Connection closed by peer." << endl;
            removeConnection(connection->getSocketFd());
            break;
        } else {
            ret_code = request.parseRequest(buffer, recv_size);
        }
    }
}

void HttpServer::run(int thread_num, int max_queue_size, int timeout) {
    if (timeout < 0) {
        cout << "HttpServer: Invalid timeout, set to default value: 3000ms" << endl;
        timeout = 3000;
    }
    ThreadPool<ConnectionSocket> thread_pool(thread_num, max_queue_size);
    listen_socket.bind();
    listen_socket.listen();
    int ret = epoll_manager->addFd(listen_socket.getSocketFd(), EpollManager::Multi_Times_Event);
    if (ret < 0) {
        cout << "HttpServer: Failed to add listen socket to epoll pool in file <"
             << __FILE__ << "> "<< "at " << __LINE__  << endl;
        exit(-1);
    }
    for (; ; ) {
        // 常引用临时变量
        const auto& requests = epoll_manager->poll(*this, timeout);
        // 遍历所有需要处理的连接的 socket_fd
        for (int socket_fd : requests) {
            cout << socket_fd << "called" << endl;
            auto connection_ptr = connection_map[socket_fd];
            cout << "real fd: " << connection_ptr->getSocketFd() << endl;
            connection_ptr->removeTimer();
            cout << "real fd: " << connection_ptr->getSocketFd() << endl;
            thread_pool.append(connection_ptr, std::bind(&HttpServer::handleRequest, this, std::placeholders::_1));
        }
        timer_manager.checkExpire();
    }
}