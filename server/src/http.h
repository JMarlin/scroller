#ifndef HTTP_H
#define HTTP_H

#include "expandobuf.h"
#include "bufloader.h"

typedef struct HttpHeader_S {
    char* Key;
    char* Value;
} HttpHeader;

typedef struct HttpRequest_S {
    ExpandoBuf rawBuf;
    char* method;
    char* path;
    char* protocol;
    int headerCount;
    HttpHeader* headers;
    char* body;
} HttpRequest;

typedef struct HttpResponse_S {
    ExpandoBuf rawBuf;
    char* protocol;
    int responseCode;
    char* responseDescription;
    int headerCount;
    HttpHeader* headers;
    char* body;
} HttpResponse;

void HttpResponse_send(int socket, HttpResponse* response);
HttpRequest HttpRequest_ReadHeaders(HttpRequest req, BufLoader* buf);
HttpRequest ReadHttpRequest(BufLoader loader);
void HttpRequest_Print(HttpRequest req);
void intToStrN(int value, char* valueStrBuf, size_t bufSize);
void calcContentLengthStrN(char* content, char* lengthStrBuf, size_t bufSize);
char* bufferInsertString(char* data, char* buf);

#endif //HTTP_H