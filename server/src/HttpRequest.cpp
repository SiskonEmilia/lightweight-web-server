#include <sstream>
#include <iostream>

#include "http/HttpRequest.h"

const std::unordered_map<std::string, HttpRequest::HTTP_HEADER> HttpRequest::header_map = {
    {"HOST", Host},
    {"USER-AGENT", User_Agent},
    {"CONNECTION", Connection},
    {"ACCEPT-ENCODING", Accept_Encoding},
    {"ACCEPT-LANGUAGE", Accept_Language},
    {"ACCEPT", Accept},
    {"CACHE-CONTROL", Cache_Control},
    {"CONTENT-LENGTH", Content_Length},
    {"UPGRADE-INSECURE-REQUESTS", Upgrade_Insecure_Requests}
};

HttpRequestParser::HTTP_CODE
HttpRequest::parseRequest(char *buffer, int buffer_size) {

    #ifdef DEBUG_MODE
    std::cout << "Buffer_size: " << buffer_size << std::endl;
    buffer[buffer_size] = '\0';
    std::cout << "Buffer:\n" << buffer << std::endl;
    // std::cout << *this << std::endl;
    #endif

    HttpRequestParser::HTTP_CODE ret_code =
        HttpRequestParser::parseRequest(buffer, buffer_size, getBuffer(), buffer_index, this->buffer_size, *this);
    if (ret_code != HttpRequestParser::HTTP_CODE::No_Request) {
        // 已经完成解析，不需要再持有临时缓冲区
        #ifdef DEBUG_MODE
        std::cout << "Finished" << std::endl;
        std::cout << *this << std::endl;
        #endif
        temp_buffer.reset();
    }
    return ret_code;
}

void HttpRequest::clear() {
    parse_state = HttpRequestParser::Parse_Request_Line;
    http_version = VERSION_NOT_SUPPORT;
    http_method = METHOD_NOT_SUPPORT;
    http_body.clear();
    http_uri.clear();
    http_headers.clear();
    http_content_length = 0;
    buffer_index = 0;
}

#ifdef DEBUG_MODE
std::ostream &operator<<(std::ostream &os, const HttpRequest &http_request) {
    std::stringstream ss;
    ss << "Parsed Http_Request Information\n"
       << "Http version: " << http_request.http_version << "\n"
       << "Http method: " << http_request.http_method << "\n" 
       << "Uri: " << http_request.http_uri << "\n"
       << "Body: " << http_request.http_body << "\n"
       << "Length: " << http_request.http_content_length << std::endl;
    os << ss.str() << std::endl;
    return os;
}
#endif