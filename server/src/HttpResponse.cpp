#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "http/HttpResponse.h"
#include "utils/Utils.h"
#include "fs/FileManager.h"

const std::unordered_map<std::string, std::string> HttpResponse::mime_map = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {"", "text/plain"}
};

const std::unordered_map<int, std::string> HttpResponse::status_map = {
    {200, "OK"},
    {400, "Bad Request"},
    {404, "Not Found"},
    {500, "Internal Server Error"}
};

void HttpResponse::clear() {
    headers.clear();
    http_version = HttpRequest::HTTP_VERSION::VERSION_NOT_SUPPORT;
    status_code = OK;
    response_mode = File;
    file_path.clear();
    mime_type = "text/plain";
    response_header.clear();
    body_buffer.reset();
    file_manager.reset();
    body_size = 0;
    sent_size = 0;
    keep_alive = false;
}

void HttpResponse::setStatusCode(HttpStatusCode new_status_code) {
    status_code = new_status_code;
    if (status_map.find(status_code) == status_map.end()) {
        std::cout << "HttpResponse: Unsupported status code: " << status_code << std::endl;
        status_code = Internal_Server_Error;
    }
}

const std::string& HttpResponse::pourHeader() {
    if (http_version == HttpRequest::HTTP_VERSION::VERSION_NOT_SUPPORT || response_header.size() != 0) {
        // 不支持的版本，返回空对象来通知服务器关闭连接 或者 已经在发送阶段，不能重复处理
        return response_header;
    }

    // 检查文件存在性及访问权限
    int file_fd = -1;
    if (status_code == OK && response_mode == File) {
        if (access(file_path.c_str(), R_OK)) {
            status_code = Not_Found;
        } else {
            file_fd = open(file_path.c_str(), O_RDONLY);
            if (file_fd < 0) {
                status_code = Not_Found;
            }
        }
    }

    /* response line */
    response_header = "HTTP/";
    // version
    if (http_version == HttpRequest::HTTP_VERSION::HTTP_10) {
        response_header += "1.0 ";
    } else {
        response_header += "1.1 ";
    }
    
    // check status code
    auto iter = status_map.find(status_code);
    if (iter == status_map.end()) {
        status_code = Internal_Server_Error;
        iter = status_map.find(Internal_Server_Error);
    }

    // status code
    response_header += std::to_string(status_code);
    response_header += " ";
    // reason phase
    response_header += iter->second;
    response_header += "\r\n";
    /* end of response line */

    /* response header */
    // add content-length header
    if (status_code != OK) {
        addHeader("Content-length", "0");
    } else if (response_mode == File) {
        // 设置 MIME Header
        std::size_t dot_pos = file_path.find_last_of(".");
        if (dot_pos != std::string::npos) {
            auto mime_iter = mime_map.find(file_path.substr(dot_pos));
            if (mime_iter != mime_map.end()) {
                mime_type = mime_iter->second;
            }
        }
        addHeader("Content-Type", mime_type);
        // 打开文件，防止写入
        struct stat st;
        stat(file_path.c_str(), &st);
        body_size = st.st_size;
        std::shared_ptr<FileManager> new_file_manager(new FileManager(file_fd, body_size));
        file_manager.swap(new_file_manager);
        addHeader("Content-length", std::to_string(body_size));
    } else {
        addHeader("Content-length", std::to_string(body_size));
    }
    // headers
    for (auto iter : headers) {
        response_header += iter.first;
        response_header += ": ";
        response_header += iter.second;
        response_header += "\r\n";
    }
    response_header += "\r\n";
    /* end of response header */

    return response_header;
}