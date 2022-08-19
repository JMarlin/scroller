#include "expandobuf.h"

#include <stdlib.h>

ExpandoBuf ExpandoBuf_new(int elemSize) {
    return (ExpandoBuf) { 0, 0, elemSize };
}

ExpandoBuf ExpandoBuf_Expand(ExpandoBuf buf) {
    buf.capacity = buf.capacity == 0 ? buf.elemSize : (buf.capacity * 2);
    buf.data = (void*)realloc(buf.data, buf.capacity);

    if(buf.data == (void*)0) buf.capacity = 0;

    return buf;
}

ExpandoBuf ExpandoBuf_ExpandTo(ExpandoBuf buf, int targetCapacity) {

    while(buf.capacity < (targetCapacity * buf.elemSize)) {
        buf = ExpandoBuf_Expand(buf);
    }

    return buf;
}