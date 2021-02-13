#pragma once

#include <memory>
#include <string>

namespace Utils {
    std::string& trim(std::string& str);
    std::shared_ptr<char> urlDecode(const std::string& url);

    template<typename T>
    void arrayDeleter(T* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }
    struct EnumClassHash {
        template<typename T>
        std::size_t operator()(T t) const {
            return static_cast<std::size_t>(t);
        }
    };
};