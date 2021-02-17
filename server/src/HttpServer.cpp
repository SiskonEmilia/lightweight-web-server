#include <iostream>
#include <algorithm>

#include "config/Configs.h"
#include "http/HttpServer.h"
#include "timer/HttpTimer.h"
#include "sockets/ConnectionSocket.h"
#include "fs/FileManager.h"

using std::cout;
using std::endl;

// const std::regex HttpServer::ServeRule::path_checker("^/[a-z0-9_./*]*");

void HttpServer::defaultRequestCallback(HttpData &data) {
    data.response.setStatusCode(HttpResponse::Not_Found);
    data.response.setResponseMode(HttpResponse::Buffer);
    data.response.setVersion(data.request.http_version);
}

int HttpServer::sendLoop(const int socket_fd, const char* buffer, int size_to_send) {
    int sent_size_sum = 0;
    int sent_size = -1;
    while (size_to_send > 0) {
        sent_size = send(socket_fd, buffer, size_to_send, 0);
        #ifdef DEBUG_VERSION
        cout << sent_size << endl;
        #endif
        if (sent_size > 0) {
            sent_size_sum += sent_size;
            size_to_send -= sent_size;
            buffer += sent_size;
        } else if (sent_size == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                #ifdef ENABLE_LOG
                cout << "HttpServer: Known error occurred while sending. Errno: " << errno << endl;
                #endif
                return sent_size_sum;
            }
            #ifdef ENABLE_LOG
            cout << "HttpServer: Unknown error occurred while sending. Errno: " << errno << endl;
            #endif
            return -2; 
        } else {
            #ifdef ENABLE_LOG
            cout << "HttpServer: Connection closed by peer while sending. " << errno << endl;
            #endif
            return -2;
        }
    }
    return sent_size_sum;
}

void HttpServer::handleSending(std::shared_ptr<ConnectionSocket> connection) {
    auto &request  = connection->http_data.request;
    auto &response = connection->http_data.response;

    // 发送数据
    {
        if (response.getSentSize() < response.pourHeader().size()) {
            int sent_size = response.getSentSize();
            const std::string &buffer = response.pourHeader();

            sent_size = sendLoop(connection->getSocketFd(), buffer.c_str() + sent_size, buffer.size() - sent_size);
            if (sent_size == 0 || sent_size == -2) {
                removeConnection(connection->getSocketFd());
                return;
            }

            response.addSentSize(sent_size);
            if (response.getSentSize() < response.pourHeader().size()) {
                connection->setTimer(3000);
                epoll_manager->modFd(connection->getSocketFd(), EpollManager::Single_Time_Send_Event);
                return;
            }
        }
        if (response.getBufferSentSize() < response.getBodySize()) {
            int sent_size = response.getBufferSentSize();
            if (response.getMode() == HttpResponse::File) {
                auto file_ptr = response.pourFile();
                if (file_ptr && file_ptr->getPtr() != nullptr) {
                    sent_size = sendLoop(connection->getSocketFd(), static_cast<const char *>(file_ptr->getPtr()) + sent_size,
                        response.getBodySize() - sent_size);
                    if (sent_size == 0 || sent_size == -2) {
                        removeConnection(connection->getSocketFd());
                        return;
                    }
                } else {
                    #ifdef ENABLE_LOG
                    cout << "HttpServer: Failed to map file to memory." << endl;
                    #endif
                    response.setKeepAlive(false);
                }
            } else {
                auto buffer_ptr = response.pourBuffer();
                if (buffer_ptr) {
                    sent_size = sendLoop(connection->getSocketFd(), buffer_ptr.get() + sent_size, response.getBodySize() - sent_size);
                    if (sent_size == 0 || sent_size == -2) {
                        removeConnection(connection->getSocketFd());
                        return;
                    }
                } else {
                    #ifdef ENABLE_LOG
                    cout << "HttpServer: Failed to get buffer from response manager." << endl;
                    #endif
                    response.setKeepAlive(false);
                }
            }
            response.addSentSize(sent_size);
            if (response.getBufferSentSize() < response.getBodySize()) {
                connection->setTimer(3000);
                epoll_manager->modFd(connection->getSocketFd(), EpollManager::Single_Time_Send_Event);
                return;
            }
        }
    }
    

    if (response.getKeepAlive()) {
        request.clear();
        response.clear();
        connection->setTimer(20000);
        epoll_manager->modFd(connection->getSocketFd(), EpollManager::Single_Time_Accept_Event);
    } else {
        removeConnection(connection->getSocketFd());
    }
}

std::shared_ptr<HttpTimer> 
HttpServer::createTimer(int timeout, int socket_fd) {
    MutexLockGuard guard(this->connection_map_mutex);
    auto iter = connection_map.find(socket_fd);
    if (iter == connection_map.end()) {
        #ifdef ENABLE_LOG
        cout << "HttpServer: Trying to create timer for the connection not existed." << endl;
        #endif
        return std::shared_ptr<HttpTimer>();
    }
    std::shared_ptr<HttpTimer> timer(new HttpTimer(timeout, iter->second));
    timer_manager.append(std::static_pointer_cast<Timer>(timer));
    return timer;
}

void HttpServer::handleConnection(int timeout) {
    std::shared_ptr<ConnectionSocket> connection(new ConnectionSocket(*this));
    while(listen_socket.accept(*connection) > 0) {
        {
            MutexLockGuard fd_guard(this->fd_cnt_mutex);
            if (--valid_fd_cnt < 0) {
                ++valid_fd_cnt;
                connection->close();
                continue;
            }
        }
        connection->setNonBlocking();
        int ret = epoll_manager->addFd(connection->getSocketFd(), EpollManager::Single_Time_Accept_Event);
        if (ret < 0) {
            #ifdef ENABLE_LOG 
            // 添加事件失败，打印错误报告
            cout << "HttpServer: Failed to handle connection for socket fd: " 
                 << connection->getSocketFd() << endl;
            #endif
        } else {
            {
                MutexLockGuard guard(this->connection_map_mutex);
                connection_map[connection->getSocketFd()] = connection;
            }
            connection->setTimer(timeout);
        }
        std::shared_ptr<ConnectionSocket> new_connection(new ConnectionSocket(*this));
        connection.swap(new_connection);
    }
}

void HttpServer::removeConnection(const int socket_fd) {
    MutexLockGuard guard(this->connection_map_mutex);
    auto iter = connection_map.find(socket_fd);
    if (iter == connection_map.end()) {
        #ifdef ENABLE_LOG
        cout << "HttpServer: Failed to find connection to remove. Socket_fd: " << socket_fd << endl;
        #endif
        return;
    }
    iter->second->removeTimer();
    epoll_manager->delFd(socket_fd);
    connection_map.erase(iter);
    MutexLockGuard fd_guard(this->fd_cnt_mutex);
    ++valid_fd_cnt;
}

void HttpServer::handleRequest(std::shared_ptr<ConnectionSocket> connection) {
    // 注意 buffer 要预留 \0 的位置
    #ifdef ENABLE_LOG
    cout << "HttpServer: Handling Request with socket_fd: " << connection->getSocketFd() << endl;
    #endif
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
                    auto &response = connection->http_data.response;
                    // 调用用户定义的回调函数
                    bool rule_found = false;
                    for (const auto& rule: get_rules) {
                        if (std::regex_match(request.http_path, rule.path)) {
                            rule.callback(connection->http_data);
                            rule_found = true;
                            break;
                        }
                    }

                    // 设置长连接情况
                    auto header_iter = request.http_headers.find(HttpRequest::HTTP_HEADER::Connection);
                    if (header_iter != request.http_headers.end() && header_iter->second == "keep-alive") {
                        response.addHeader("keep-alive", "timeout=20");
                        response.setKeepAlive(true);
                    }

                    // 处理未调用的情况
                    if (!rule_found) {
                        defaultRequestCallback(connection->http_data);
                        cout << "HttpServer: Failed to match rule for: " << request.http_path << endl;
                    }
                    handleSending(connection);
                    break;
                } else {
                    // Wait for following msg
                    connection->setTimer(3000);
                    epoll_manager->modFd(connection->getSocketFd(), EpollManager::Single_Time_Accept_Event);
                    break;
                }
            } else if (errno == EWOULDBLOCK) {
                connection->setTimer(3000);
                epoll_manager->modFd(connection->getSocketFd(), EpollManager::Single_Time_Accept_Event);
                break;
            } else {
                #ifdef ENABLE_LOG
                cout << "HttpServer: Unknown ERRNO: " << errno << endl; 
                #endif
                removeConnection(connection->getSocketFd());
            }
            break;
        } else if (recv_size == 0) {
            // Connection closed by peer
            removeConnection(connection->getSocketFd());
            break;
        } else {
            ret_code = request.parseRequest(buffer, recv_size);
        }
    }
}


void HttpServer::Get(const std::regex &path, const RequestCallback& callback) {
    get_rules.push_back({path, callback});
}

void HttpServer::run(int thread_num, int max_queue_size, int timeout) {
    if (timeout < 0) {
        cout << "HttpServer: Invalid timeout, set to default value: 3000ms" << endl;
        timeout = 3000;
    }
    ThreadPool<ConnectionSocket> thread_pool(thread_num, max_queue_size);
    listen_socket.bind();
    listen_socket.listen();
    int ret = epoll_manager->addFd(listen_socket.getSocketFd(), EpollManager::Multi_Times_Accept_Event);
    if (ret < 0) {
        cout << "HttpServer: Failed to add listen socket to epoll pool in file <"
             << __FILE__ << "> "<< "at " << __LINE__  << endl;
        exit(-1);
    }
    for (; ; ) {
        // 常引用临时变量
        const auto& requests = epoll_manager->poll(*this, timeout);
        #ifdef ENABLE_LOG
        cout << "HttpServer: Pool returned: " << requests.size() << endl;
        #endif
        // 遍历所有需要处理的连接的 socket_fd
        auto connection_iter = connection_map.end();
        for (int socket_fd : requests) {
            MutexLockGuard guard(this->connection_map_mutex);
            if ((connection_iter = connection_map.find(socket_fd)) == connection_map.end()) {
                #ifdef ENABLE_LOG
                cout << "HttpServer: Epoll returned removed connection." << endl;
                #endif
                continue;
            }
            auto &connection_ptr = connection_iter->second;
            connection_ptr->removeTimer();
            if (connection_ptr->http_data.response.getSentSize() == 0) {
                #ifdef DEBUG_VERSION
                cout << "socket: " << connection_ptr->getSocketFd() << " now handling request." << endl;
                #endif
                thread_pool.append(connection_ptr, std::bind(&HttpServer::handleRequest, this, std::placeholders::_1));
            } else {
                #ifdef DEBUG_VERSION
                cout << "socket: " << connection_ptr->getSocketFd() << " now handling response." << endl;
                #endif
                thread_pool.append(connection_ptr, std::bind(&HttpServer::handleSending, this, std::placeholders::_1));
            }
        }
        timer_manager.checkExpire();
    }
}