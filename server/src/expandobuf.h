#ifndef EXPANDOBUF_H
#define EXPANDOBUF_H

#include <stdlib.h>

typedef struct ExpandoBuf_S {
    void* data;
    int capacity;
    size_t elemSize;
} ExpandoBuf;

ExpandoBuf ExpandoBuf_new(int elemSize);
ExpandoBuf ExpandoBuf_Expand(ExpandoBuf buf);
ExpandoBuf ExpandoBuf_ExpandTo(ExpandoBuf buf, int targetCapacity);

#endif //EXPANDOBUF_H