#include "middleware/StaticFile.h"
#include "http/HttpData.h"

const std::regex StaticFile::mount_path_validator = std::regex("^/[a-zA-Z0-9/]*");

void StaticFile::callback(HttpData &data) {
    auto &request  = data.request;
    auto &response = data.response;

    response.setVersion(request.http_version);

    std::string file_path = base_path + request.http_path.substr(mount_path.size());
    if (file_path.find("..") != std::string::npos){
        std::cout << "StaticFile: HACK NOT ALLOWED" << std::endl;
        response.setStatusCode(HttpResponse::Not_Found);
    } else if (access(file_path.c_str(), R_OK) == 0) {
        response.setStatusCode(HttpResponse::OK);
        response.setResponseMode(HttpResponse::File);
        response.setFilePath(file_path);
    } else {
        response.setStatusCode(HttpResponse::Not_Found);
    }
}