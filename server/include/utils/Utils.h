#pragma once

namespace Utils {
    template<typename T>
    void arrayDeleter(T* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }
};