#pragma once

class HttpRequest;
/**
 * @brief 解析 Http 请求
 * 
*/
class HttpRequestParser { 
public:
    enum HTTP_CODE {
        No_Request = 0, Get_Request, Bad_Request, Forbidden_Request, Internal_Error,
        Closed_Connection
    };
    enum LINE_STATE {
        Line_OK = 0, Line_Bad, Line_More, Line_Overflow
    };
    enum PARSE_STATE {
        Parse_Request_Line = 0, Parse_Header, Parse_Body
    };
    /**
     * @brief 解析一行，将其拷贝到临时缓存中，视情况返回解析状态。解析完成后，
     *  start_index 和 temp_start_index 分别指向两个 buffer 的首个未读位置。
     * @return 返回 Line_Bad 说明有异常的结束符
    */
    static LINE_STATE checkLine(char *buffer, int &start_index, int buffer_size,
        char *temp_buffer, int &temp_start_index ,int temp_buffer_size);
    /**
     * @brief 从临时缓存解析请求行
    */
    static HTTP_CODE parseRequestLine(char *temp_buffer, HttpRequest &http_request);
    /**
     * @brief 从临时缓存解析请求头的一行
    */
    static HTTP_CODE parseHeaderLine(char *temp_buffer, HttpRequest &http_request);
    /**
     * @brief 从缓存拷贝请求体
    */
    static HTTP_CODE parseBody(char *buffer, int buffer_size, HttpRequest &http_request);
    /**
     * @brief 解析请求内容，存入 HttpRequest 类中
    */
    static HTTP_CODE parseRequest(char *buffer, int buffer_size,
        char *temp_buffer, int &temp_buffer_index, int temp_buffer_size,
        HttpRequest &http_request);
};