#include "utils/Utils.h"

std::string& Utils::trim(std::string& str) {
    if (str.empty()) return str;
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
}

std::shared_ptr<char> Utils::urlDecode(const std::string& url) {
    int size = url.size();
    for (auto ch: url) {
        if (ch == '%' || ch == '+') {
            size -= 2;
        }
    }
    std::shared_ptr<char> ret(new char[size + 1], arrayDeleter<char>);
    for (int ret_index = 0, url_index = 0; ret_index < size; ++ret_index, ++url_index) {
        if (url[url_index] == '+') {
            ret.get()[ret_index] = ' ';
        } else if (url[url_index] == '%') {
            ret.get()[ret_index] = 
                std::stoi(url.substr(++url_index, 2), nullptr, 16);
            ++url_index;
        } else {
            ret.get()[ret_index] = url[url_index];
        }
    }
    ret.get()[size] = '\0';
    return ret;
}