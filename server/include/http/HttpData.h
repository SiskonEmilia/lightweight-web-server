#pragma once

#include <memory>
#include <http/HttpRequest.h>
#include <http/HttpResponse.h>

typedef struct HttpData {
    HttpRequest  request;
    HttpResponse response;
} HttpData;