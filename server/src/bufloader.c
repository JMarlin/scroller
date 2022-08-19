#include "bufloader.h"
#include <stdio.h>
#include <unistd.h>

int BufLoader_loadAtLeastNext(BufLoader* loader, int count) {
    int requested = loader->read + count;

    if(loader->available >= requested) return 1;

    int result = loader->loadChunk(loader, requested);

    loader->eof = loader->available <= requested;

    if(result) {
        loader->read += count;

        return 1;
    } else {

        printf("BUFFER UNDERRUN!");
        return 0;
    }
}

char BufLoader_peek(BufLoader* loader) {
    if(!BufLoader_loadAtLeastNext(loader, 2)) return 0;

    return loader->current[1];
}

char BufLoader_next(BufLoader* loader) {
    if(!BufLoader_loadAtLeastNext(loader, 1)) return 0;

    loader->current = &((char*)loader->targetBuf.data)[loader->currentOffset++];

    return *loader->current;
}

void BufLoader_readUntil(BufLoader* loader, char* delims) {
    while(1) {
        char c = BufLoader_next(loader);
        char* d;
        if(loader->eof) break;
        for(d = delims; *d && *d != c; d++) {}
        if(*d) break;
    }
}

BufLoader BufLoaderBase() {
    BufLoader loader = { 0 };

    loader.targetBuf.elemSize = sizeof(char);

    return loader;
}

void BufLoader_skipWhitespace(BufLoader* loader) {
    while(1) {
        if(loader->eof || BufLoader_peek(loader) > 0x20) return;
        BufLoader_next(loader);
    }
}

#define LBFS_CHUNKSZ 2048
int loadBufFromSocket(BufLoader* genericThis, int requested) {

    SocketBufLoader* this = (SocketBufLoader*)genericThis;
    int read_count = LBFS_CHUNKSZ;

    while(read_count == LBFS_CHUNKSZ && this->available < requested) {

        this->targetBuf = ExpandoBuf_ExpandTo(this->targetBuf, this->targetBuf.capacity + LBFS_CHUNKSZ);

        read_count = read(this->socket, &((char*)this->targetBuf.data)[this->currentOffset], read_count);

        this->available += read_count;
    }

    return this->available >= requested;
}

BufLoader SocketLoader(int socket) {
    BufLoader loader = BufLoaderBase();
    SocketBufLoader* socketLoader = (SocketBufLoader*)&loader;

    socketLoader->loadChunk = &loadBufFromSocket;
    socketLoader->socket = socket;

    return loader;
}