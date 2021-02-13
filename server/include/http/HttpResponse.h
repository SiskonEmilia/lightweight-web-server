#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "http/HttpRequest.h"

class FileManager;

class HttpResponse {
public:
    /**
     * @brief 不同的响应模式
    */
    enum ResponseMode {
        File,   // 默认模式，此模式下，系统将尝试通过设置的文件路径读取作为响应内容，若未找到文件，则返回 404
        Buffer  // 此模式下，系统将会发送 Buffer 内存在的内容，此模式需要设置 MIME 类型
    };
    /**
     * @brief HTTP 状态码
    */
    enum HttpStatusCode {
        OK = 200,
        Bad_Request = 400,
        Not_Found = 404,
        Internal_Server_Error = 500
    };

private:
    const static std::unordered_map<std::string, std::string> mime_map;
    const static std::unordered_map<int, std::string> status_map;
    std::unordered_map<std::string, std::string> headers;
    // FIXME: 不恰当的编译依赖，后续可以单独开辟一个文件定义这个枚举类型
    HttpRequest::HTTP_VERSION http_version;
    HttpStatusCode status_code;
    ResponseMode   response_mode;
    std::string    file_path;
    std::string    mime_type;
    std::string    response_header;
    std::shared_ptr<char> body_buffer;
    std::shared_ptr<FileManager> file_manager;
    int body_size;
    int sent_size;
    bool keep_alive;

public:
    HttpResponse() : headers(), http_version(HttpRequest::HTTP_VERSION::VERSION_NOT_SUPPORT),
        status_code(OK), response_mode(File), file_path(), mime_type("text/plain"),
        response_header(), body_buffer(), file_manager(), body_size(0), sent_size(0), keep_alive(false) { }

    void clear();

    /**
     * @brief 为 Header 添加键值对
    */
    void addHeader(const std::string &header_key, const std::string &header_val) {
        headers[header_key] = header_val;
    }

    void setVersion(HttpRequest::HTTP_VERSION new_version) {
        http_version = new_version;
    }

    void setStatusCode(HttpStatusCode new_status_code);

    void setResponseMode(ResponseMode new_response_mode) {
        response_mode = new_response_mode;
    }

    void setBuffer(std::shared_ptr<char> new_buffer, int buffer_size) {
        body_buffer = new_buffer;
        body_size = buffer_size;
    }

    void setFilePath(const std::string &new_file_path) {
        file_path = new_file_path;
    }

    void setMimeType(const std::string &new_mime_type) {
        mime_type = new_mime_type;
    }

    const int getBodySize() {
        return body_size;
    }

    const ResponseMode getMode() {
        return response_mode;
    }

    const int getSentSize() {
        return sent_size;
    }

    void addSentSize(const int new_sent_size) {
        sent_size += new_sent_size;
    }

    const int getBufferSentSize() {
        return sent_size - response_header.size();
    }

    const bool getKeepAlive() {
        return keep_alive;
    }

    void setKeepAlive(const bool new_keep_alive) {
        keep_alive = new_keep_alive;
    }

    /**
     * @brief 
     * @return 
    */
    const std::string& pourHeader();
    
    std::shared_ptr<FileManager> pourFile() {
        return file_manager;
    }
    
    std::shared_ptr<char> pourBuffer() {
        return body_buffer;
    }
};