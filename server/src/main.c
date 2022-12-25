#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#include "webobject.h"
#include "router.h"
#include "http.h"
#include "util.h"

#define DEFAULT_PORT "8080"

//TODO
// - WebData
//   - Implement parse from string buf
// - Load body content to buffer using the incoming 'Content-Length' header value
// - Implement http client functionality
// - Implement 'default' http response construction
// - Implement dynamically adding headers to a request or response
// - Detect socket closed during buffer load/read
// - Threadify accept loop

HttpRequest GetHttpRequest(int clientsock) {

    printf("Getting http request...\n");

    HttpRequest req = ReadHttpRequest(SocketLoader(clientsock));

    printf("Done getting request\n");

    return req;
}

void HttpSendStatic(int socket, char* filePath, char* mimetype) {
    char lengthStr[32];
    FILE* file = fopen(filePath, "rb");

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    intToStrN(length, lengthStr, 32);

    HttpResponse res = (HttpResponse){
        .protocol = "HTTP/1.1",
        .responseCode = 200,
        .responseDescription = "OK",
        .headerCount = 4,
        .headers = (HttpHeader[]){
            { "Content-Type", mimetype },
            { "Content-Length", lengthStr },
            { "Cross-Origin-Opener-Policy", "same-origin" },
            { "Cross-Origin-Embedder-Policy", "require-corp" }
        },
        .body = ""
    };

    HttpResponse_send(socket, &res);

    unsigned char buffer[2048];

    while(write(socket, buffer, fread(buffer, 1, 2048, file)) == 2048);

    fclose(file);
}

void IndexHandler(HttpRequest* request, int socket) {
    HttpSendStatic(socket, "../client/index.html", "text/html");
}

void StaticJSHandler(HttpRequest* request, int socket) {
    HttpSendStatic(socket, "../client/client.js", "application/javascript");
}

void StaticWASMHandler(HttpRequest* request, int socket) {
    HttpSendStatic(socket, "../client/client.wasm", "application/wasm");
}

void StaticGameWASMHandler(HttpRequest* request, int socket) {
    printf("\nSTATIC GAME WASM HANDLER\n\n");
    HttpSendStatic(socket, "../client/game.wasm", "application/wasm");
}

void NotFoundHandler(HttpRequest* request, int socket) {
    const char* template = "<center><h1>404: path %s not found</h1></center>";
    char contentLengthStr[32];
    int length = strlen(request->path) + strlen(template) - 2;
    char* bodyMessage = (char*)malloc(length + 1);

    snprintf(bodyMessage, length + 1, template, request->path);

    calcContentLengthStrN(bodyMessage, contentLengthStr, 32);

    HttpResponse res = (HttpResponse){
        .protocol = "HTTP/1.1",
        .responseCode = 404,
        .responseDescription = "NOTFOUND",
        .headerCount = 2,
        .headers = (HttpHeader[]){
            { "Content-Type", "text/html" },
            { "Content-Length", contentLengthStr }
        },
        .body = bodyMessage
    };

    HttpResponse_send(socket, &res);
    free(bodyMessage);
}

int main(int argc, char* argv[]) {

    EndpointCollection endpoints = (EndpointCollection) {
        .endpointCount = 4,
        .endpoints = (Endpoint[]) {
            { .path = "/", .method = "GET", .handler = IndexHandler },
            { .path = "/client.js", .method = "GET", .handler = StaticJSHandler },
            { .path = "/client.wasm", .method = "GET", .handler = StaticWASMHandler },
            { .path = "/game.wasm", .method = "GET", .handler = StaticGameWASMHandler }
        },
        .notFoundHandler = NotFoundHandler
    };

    char* portString = getenv("FUNCTIONS_CUSTOMHANDLER_PORT");

    if(!portString)
        portString = DEFAULT_PORT;

    printf("The port is: %s\n", portString);

    int listensock, clientsock;
    socklen_t len;
    struct sockaddr_in servaddr, cli;
   
    // socket create and verification
    listensock = socket(AF_INET, SOCK_STREAM, 0);
    if (listensock == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(atoi(portString));
   
    // Binding newly created socket to given IP and verification
    if ((bind(listensock, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully bound..\n");

    // Now server is ready to listen and verification
    if ((listen(listensock, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");

    len = sizeof(cli);

    while(1) {
        // Accept the data packet from client and verification
        clientsock = accept(listensock, (struct sockaddr*)&cli, &len);
        if (clientsock < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else
            printf("server accept the client...\n");
    
        HttpRequest req = GetHttpRequest(clientsock);
        HttpRequest_Print(req);

        RequestHandler targetHandler = NotFoundHandler;
        for(int i = 0; i < endpoints.endpointCount; i++) {
            if(!strcasecmp(endpoints.endpoints[i].path, req.path) && !strcasecmp(endpoints.endpoints[i].method, req.method)) {
                targetHandler = endpoints.endpoints[i].handler;
                break;
            }
        }

        targetHandler(&req, clientsock);

        free(req.rawBuf.data);
        free(req.headers);
   
        close(clientsock);

        printf("Done, waiting for next request...\n");
    }

    return 0;
}