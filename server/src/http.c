#include "http.h"
#include "expandobuf.h"
#include "bufloader.h"
#include "util.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void HttpResponse_send(int socket, HttpResponse* response) {
    //First line
    write(socket, response->protocol, strlen(response->protocol));
    write(socket, " ", 1);
    int digits = decimalDigits(response->responseCode);
    char* number = (char*)malloc(digits + 1);
    snprintf(number, digits + 1, "%d", response->responseCode);
    write(socket, number, digits);
    free(number);
    write(socket, " ", 1);
    write(socket, response->responseDescription, strlen(response->responseDescription));
    write(socket, "\r\n", 2);

    //Headers
    for(int i = 0; i < response->headerCount; i++) {
        write(socket, response->headers[i].Key, strlen(response->headers[i].Key));
        write(socket, ": ", 2);
        write(socket, response->headers[i].Value, strlen(response->headers[i].Value));
        write(socket, "\r\n", 2);
    }

    write(socket, "\r\n", 2);

    //TODO: Actually use content length header
    write(socket, response->body, strlen(response->body));
}

HttpRequest HttpRequest_ReadHeaders(HttpRequest req, BufLoader* buf) {
    ExpandoBuf headerData = ExpandoBuf_new(sizeof(HttpHeader));
    int justSawLinebreak = 0;
    
    req.headerCount = 0;

    while(!(*buf->current == '\r' && BufLoader_peek(buf) == '\n')) {
        headerData = ExpandoBuf_ExpandTo(headerData, req.headerCount + 1);

        if(!headerData.data) {

            req.headerCount = -1;

            return req;
        }

        HttpHeader* headers = (HttpHeader*)headerData.data;

        headers[req.headerCount].Key = buf->current;

        while(BufLoader_next(buf) != ':') { }
        *(buf->current) = 0;
        while(BufLoader_next(buf) == ' ') { }

        headers[req.headerCount].Value = buf->current;

        while(*buf->current != '\r' && BufLoader_peek(buf) != '\n') 
            BufLoader_next(buf);

        *(buf->current) = 0;

        BufLoader_next(buf);
        BufLoader_next(buf);

        req.headerCount += 1;
    }

    BufLoader_next(buf);
    BufLoader_next(buf);

    req.headers = (HttpHeader*)headerData.data;
    req.body = buf->current;

    return req;
}

HttpRequest ReadHttpRequest(BufLoader loader) {
    printf("Reading buf to request...\n");
    HttpRequest req = { 0 };

    req.rawBuf = loader.targetBuf;

    BufLoader_next(&loader);

    req.method = loader.current;
    BufLoader_readUntil(&loader, " \r\n");
    *loader.current = 0;

    BufLoader_next(&loader);
    req.path = loader.current;
    BufLoader_readUntil(&loader, " \r\n");
    *loader.current = 0;

    BufLoader_next(&loader);
    req.protocol = loader.current;
    BufLoader_readUntil(&loader, " \r\n");
    *loader.current = 0;

    char next = '\n';

    while(next == '\r' || next == '\n') next = BufLoader_next(&loader);

    req = HttpRequest_ReadHeaders(req, &loader);

    if(req.headerCount < 0) 
        return (HttpRequest){ 0 };

    return req;
}

void intToStrN(int value, char* valueStrBuf, size_t bufSize) {
    if(bufSize == 0) return;
    if(bufSize < decimalDigits(value) + 1) { *valueStrBuf = 0; return; };

    snprintf(valueStrBuf, bufSize, "%d", value);
}

void calcContentLengthStrN(char* content, char* lengthStrBuf, size_t bufSize) {
    return intToStrN(strlen(content), lengthStrBuf, bufSize);
}


void HttpRequest_Print(HttpRequest req) {
    printf("Method: %s\n", req.method);
    printf("Path: %s\n", req.path);
    printf("Protocol: %s\n", req.protocol);
    printf("Headers:\n");

    for(int i = 0; i < req.headerCount; i++) 
        printf("    [%s] %s\n", req.headers[i].Key, req.headers[i].Value);

    printf("Body:\n");
    printf("%s\n", req.body);
}

char* bufferInsertString(char* data, char* buf) {
    int length = strlen(data);
    memcpy(buf, data, length);

    return buf + length;
}
