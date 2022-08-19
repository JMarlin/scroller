#ifndef ROUTER_H
#define ROUTER_H
#include "http.h"

typedef void (*RequestHandler)(HttpRequest*, int);

typedef struct Endpoint_S {
    char* path;
    char* method;
    RequestHandler handler;
} Endpoint;

typedef struct EndpointCollection_S {
    int endpointCount;
    Endpoint* endpoints;
    RequestHandler notFoundHandler;
} EndpointCollection;

#endif //ROUTER_H