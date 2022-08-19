#ifndef BUFLOADER_H
#define BUFLOADER_H

#include "expandobuf.h"
#include <stdlib.h>

struct BufLoader_S;

typedef int (*BufLoaderMethod)(struct BufLoader_S*, int);

typedef struct BufLoader_S {
    BufLoaderMethod loadChunk;
    ExpandoBuf targetBuf;
    size_t read;
    size_t available;
    int eof;
    int currentOffset;
    char* current;
    void* data;
} BufLoader;

typedef struct SocketBufLoader_S {
    BufLoaderMethod loadChunk;
    ExpandoBuf targetBuf;
    size_t read;
    size_t available;
    int eof;
    int currentOffset;
    char* current;
    int socket;
} SocketBufLoader;

int BufLoader_loadAtLeastNext(BufLoader* loader, int count);
char BufLoader_peek(BufLoader* loader);
char BufLoader_next(BufLoader* loader);
void BufLoader_readUntil(BufLoader* loader, char* delims);
void BufLoader_skipWhitespace(BufLoader* loader);
BufLoader BufLoaderBase();
BufLoader SocketLoader(int socket);

#endif //BUFLOADER_H