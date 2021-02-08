#pragma once

#include <memory>

#include "timer/HttpTimer.h"

class TimerManager;

class HttpData {

    std::shared_ptr<ConnectionSocket> connection_socket;

public:
    HttpData(std::shared_ptr<ConnectionSocket> connection_socket) : connection_socket(connection_socket) { }

    std::shared_ptr<ConnectionSocket>
    getSocketPtr() {
        return connection_socket;
    }
};