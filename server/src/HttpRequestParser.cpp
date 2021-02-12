#include <string>
#include <algorithm>
#include <iostream>

#include "http/HttpRequestParser.h"
#include "http/HttpRequest.h"
#include "utils/Utils.h"

using std::cout;
using std::endl;

/**
 * @brief 解析一行，将其（除了 \r\n 外）拷贝到临时缓存中，视情况返回解析状态。解析完成后，
 *  start_index 和 temp_start_index 分别指向两个 buffer 的首个未读位置。
 * @return 返回 Line_Bad 说明有异常的结束符
*/
HttpRequestParser::LINE_STATE
HttpRequestParser::checkLine(char *buffer, int &start_index, int buffer_size,
    char *temp_buffer, int &temp_start_index ,int temp_buffer_size) {
    char temp;
    while (start_index < buffer_size && temp_start_index < temp_buffer_size) {
        // 当前解析的字符
        temp = buffer[start_index];
        // 拷贝到临时缓存
        temp_buffer[temp_start_index] = temp;
        if (temp == '\r') {
            // 如果已经读到结尾
            if (++start_index == buffer_size) {
                return HttpRequestParser::LINE_STATE::Line_More;
            }
            // 如果下一位为 '\n'
            if (buffer[start_index++] == '\n') {
                if (temp_start_index == temp_buffer_size) {
                    temp_buffer[temp_start_index++] = '\0';
                    return HttpRequestParser::LINE_STATE::Line_Overflow;
                } else {
                    temp_buffer[temp_start_index++] = '\0';
                    return HttpRequestParser::LINE_STATE::Line_OK;
                }
            } else {
                // 孤立的 CR 标志符，错误行
                return HttpRequestParser::LINE_STATE::Line_Bad;
            }
        }
        ++start_index;
        ++temp_start_index;
    }
    if (temp_start_index == temp_buffer_size) {
        temp_buffer[temp_start_index++] = '\0';
        return HttpRequestParser::LINE_STATE::Line_Overflow;
    } else {
        return HttpRequestParser::LINE_STATE::Line_More;
    }
}
/**
 * @brief 从临时缓存中解析请求行
*/
HttpRequestParser::HTTP_CODE
HttpRequestParser::parseRequestLine(char *temp_buffer, HttpRequest &http_request) {
    // Request Line Format:
    // Method SP Request-URI SP HTTP-Version [CRLF]
    char *next_part  = std::strpbrk(temp_buffer, " ");
    if (next_part == nullptr) {
        // 找不到分隔符
        return Bad_Request;
    }

    // 分割 Method，方便 cmp，next_part 现在指向 RU 或者之前的 SP
    *next_part++ = '\0';
    if (strcasecmp(temp_buffer, "GET") == 0) {
        http_request.http_method = HttpRequest::HTTP_METHOD::GET;
    } else if (strcasecmp(temp_buffer, "POST") == 0) {
        http_request.http_method = HttpRequest::HTTP_METHOD::POST;
    } else if (strcasecmp(temp_buffer, "PUT") == 0) {
        http_request.http_method = HttpRequest::HTTP_METHOD::PUT;
    } else if (strcasecmp(temp_buffer, "DELETE") == 0) {
        http_request.http_method = HttpRequest::HTTP_METHOD::DELETE;
    } else {
        // No method match
        return Bad_Request;
    }

    // 跳过所有连续的空格
    next_part += strspn(next_part, " ");
    // temp_buffer 现在指向 Request-URI
    temp_buffer = next_part;
    next_part = std::strpbrk(next_part, " ");
    if (next_part == nullptr) {
        // 找不到分隔符
        return Bad_Request;
    }

    *next_part++ = '\0';
    // "http://hostname/path/..." 或者 "/path/..."
    if (strncasecmp(temp_buffer, "http://", 7) == 0) {
        temp_buffer += 7;
        temp_buffer = strchr(temp_buffer, '/');
    } else if (strncasecmp(temp_buffer, "/", 1) != 0) {
        return Bad_Request;
    }
    if (temp_buffer == nullptr || *temp_buffer != '/') {
        return Bad_Request;
    }
    http_request.http_uri = std::string(temp_buffer);

    // 跳过所有连续的空格
    // next_part 现在指向 HTTP-Version
    next_part += strspn(next_part, " ");
    if (strncasecmp("HTTP/1.1", next_part, 8) == 0) {
        http_request.http_version = HttpRequest::HTTP_11;
    } else if (strncasecmp("HTTP/1.0", next_part, 8) == 0) {
        http_request.http_version = HttpRequest::HTTP_10;
    } else {
        return Bad_Request;
    }

    http_request.parse_state = Parse_Header;
    return No_Request;
}
/**
 * @brief 从临时缓存中解析请求头的一行
*/
HttpRequestParser::HTTP_CODE
HttpRequestParser::parseHeaderLine(char *temp_buffer, HttpRequest &http_request) {
    if (*temp_buffer == '\0') {
        http_request.parse_state = Parse_Body;
        return No_Request;
    }

    char key[100], value[300];
    std::sscanf(temp_buffer, "%[^:]:%[^:]", key, value);
    std::string key_s(key);
    std::transform(key_s.begin(), key_s.end(), key_s.begin(), ::toupper);
    std::string value_s(value);

    Utils::trim(key_s);

    auto iter = HttpRequest::header_map.find(Utils::trim(key_s));
    if (iter != HttpRequest::header_map.end()) {
        http_request.http_headers.insert({iter->second, Utils::trim(value_s)});
        if (iter->second == HttpRequest::HTTP_HEADER::Content_Length) {
            http_request.http_content_length = std::stoi(value_s);
        } 
    } else {
        cout << "HttpRequestParser: Unsupported header: " << key_s << endl;
    }

    return No_Request;
}

/**
 * @brief 从缓存拷贝请求体
*/
HttpRequestParser::HTTP_CODE
HttpRequestParser::parseBody(char *buffer, int buffer_size, HttpRequest &http_request) {
    http_request.http_content_length -= buffer_size;
    if (http_request.http_content_length < 0) {
        // 过长的 body
        return Bad_Request;
    }
    buffer[buffer_size] = '\0';
    http_request.http_body += std::string(buffer);
    if (http_request.http_content_length > 0) {
        // 尚未接受完数据
        return No_Request;
    } else {
        // 已经接受完数据
        if (http_request.http_method == HttpRequest::HTTP_METHOD::GET) {
            // 只支持 GET 方法
            return Get_Request;
        }
        // 不支持的请求类型
        return Bad_Request;
    }
}
/**
 * @brief 解析请求内容，存入 HttpRequest 类中
*/
HttpRequestParser::HTTP_CODE
HttpRequestParser::parseRequest(char *buffer, int buffer_size,
    char *temp_buffer, int &temp_buffer_index, int temp_buffer_size,
    HttpRequest &http_request) {
    LINE_STATE line_state = Line_OK;
    HTTP_CODE  ret_code = No_Request;
    int start_index = 0;
    while (http_request.parse_state != Parse_Body && (line_state = 
        checkLine(buffer, start_index, buffer_size,
            temp_buffer, temp_buffer_index, temp_buffer_size)) == Line_OK) {
        cout << "TB: " << temp_buffer << endl;
        if (http_request.parse_state == PARSE_STATE::Parse_Request_Line) {
            ret_code = parseRequestLine(temp_buffer, http_request);
            if (ret_code == Bad_Request) {
                return Bad_Request;
            }
        } else {
            // parsing header
            ret_code = parseHeaderLine(temp_buffer, http_request);
            if (ret_code == Bad_Request) {
                return Bad_Request;
            }
        }
        temp_buffer_index = 0;
    }
    cout << "PS: " <<  http_request.parse_state << endl;
    if (http_request.parse_state == Parse_Body) {
        ret_code = parseBody(buffer + start_index, buffer_size - start_index, http_request);
        return ret_code;
    }
    if (line_state == Line_Bad) {
        return Bad_Request;
    } else if (line_state == Line_Overflow) {
        cout << "HttpRequestParser: Line Overflow!" << endl;
        return Bad_Request;
    } else {
        return No_Request;
    }
}