#pragma once

#include <string>
#include <functional>
#include <iostream>
#include <regex>
#include <unistd.h>

#include "http/HttpServer.h"

class StaticFile {

    std::string base_path;
    std::string mount_path;
    const static std::regex mount_path_validator;

public:
    StaticFile(std::string base_path = "./", std::string mount_path = "/")
        : base_path(base_path), mount_path(mount_path) {
        if (!std::regex_match(mount_path, mount_path_validator)) {
            std::cout << "StaticFile: Invalid mount path, set to default value: /" << std::endl;
            this->mount_path = "/";
        } else if (this->mount_path[this->mount_path.size() - 1] != '/') {
            this->mount_path += '/';
        }
        if (access(base_path.c_str(), R_OK)) {
            std::cout << "StaticFile: Invalid base path, set to default value: ./" << std::endl;
            this->base_path = "./";
        } else if (this->base_path[this->base_path.size() - 1] != '/') {
            this->base_path += '/';
        }
    }

    void callback(HttpData &data);

    std::regex getRegex() {
        return std::regex(mount_path + ".*");
    }

    HttpServer::RequestCallback getCallback() {
        return std::bind(&StaticFile::callback, this, std::placeholders::_1);
    }
};