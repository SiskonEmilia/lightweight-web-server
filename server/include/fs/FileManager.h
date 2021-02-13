#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>

/**
 * @brief RAII 风格的文件虚拟内存映射管理类
*/
class FileManager {

    int file_fd;
    int file_size;
    void *file_in_memory;

public:
    FileManager(int file_fd, int file_size) : file_fd(file_fd), file_size(file_size) {
        if (file_fd > 0 && file_size >= 0) {
            file_in_memory = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
        } else {
            std::cout << "FileManager: Trying to manage invalid file." << std::endl;
            file_in_memory = nullptr;
        }
    }
    ~FileManager() {
        if (file_in_memory != nullptr) {
            munmap(file_in_memory, file_size);
            close(file_fd);
        }
    }
    void *getPtr() {
        return file_in_memory;
    }
};