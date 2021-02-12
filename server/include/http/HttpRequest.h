#pragma once

#include <memory>
#include <cstring>
#include <unordered_map>
#include <string>

#include "http/HttpRequestParser.h"
#include "utils/Utils.h"

#define DEBUG_MODE

/**
 * 关于如何判定请求数据是否完整：
 * "If a request contains a message-body and a 
 * Content-Length is not given, the server SHOULD
 * respond with 400 (bad request) if it cannot
 * determine the length of the message, or with
 * 411 (length required) if it wishes to insist 
 * on receiving a valid Content-Length."
 * 所以如果没有检测到 Content-Length，则有 body 时直接
 * 响应 400 并关闭连接。如果有设置，则读取指定的字节数并存
 * 入对象。如果字节数相比更少，则通过 Timer 等待超时，如果
 * 字节数相比较更多，则响应 400 并关闭连接。
*/


#ifdef DEBUG_MODE
std::ostream &operator<<(std::ostream &, const HttpRequest &);
#endif

class HttpRequest {
    // enum hack
    enum {
        Default_Buffer_Size = 4096
    };

    // 临时缓冲区的大小
    int buffer_size;
    // 临时缓冲区的当前索引
    int buffer_index;
    // 用于暂存已经读入但并不完整的数据
    std::shared_ptr<char> temp_buffer; 

    char *getBuffer() {
        if (temp_buffer) {
            return temp_buffer.get();
        } else {
            std::shared_ptr<char> new_buffer(new char[buffer_size + 1], Utils::arrayDeleter<char>);
            temp_buffer.swap(new_buffer);
            bzero(temp_buffer.get(), buffer_size + 1);
            return temp_buffer.get();
        }
    }

public:
    enum HTTP_VERSION {
        HTTP_10, HTTP_11, VERSION_NOT_SUPPORT
    };

    enum HTTP_METHOD {
        GET, POST, PUT, DELETE, METHOD_NOT_SUPPORT
    };

    enum HTTP_HEADER {
        Host,
        User_Agent,
        Connection,
        Accept_Encoding,
        Accept_Language,
        Accept,
        Cache_Control,
        Content_Length,
        Upgrade_Insecure_Requests
    };

    // 当前解析状态，由外部负责重置
    HttpRequestParser::PARSE_STATE parse_state;
    // 运行时 Header 键 Hash 表
    static const std::unordered_map<std::string, HTTP_HEADER> header_map;
    HTTP_VERSION http_version;
    HTTP_METHOD  http_method;
    std::string  http_uri;
    std::string  http_body;
    int          http_content_length;
    std::unordered_map<HTTP_HEADER, std::string, Utils::EnumClassHash> http_headers;

    HttpRequest(int buffer_size = Default_Buffer_Size) 
        : buffer_size(buffer_size), buffer_index(0),
          temp_buffer(nullptr, Utils::arrayDeleter<char>),
          parse_state(HttpRequestParser::Parse_Request_Line),
          http_version(VERSION_NOT_SUPPORT), http_method(METHOD_NOT_SUPPORT),
          http_uri(), http_body(), http_content_length(0), http_headers() {
        if (buffer_size <= 0) {
            // print warning message
            this->buffer_size = Default_Buffer_Size;
        }
    }
    
    void clear();
    
    HttpRequestParser::HTTP_CODE parseRequest(char *buffer, int buffer_size);
};