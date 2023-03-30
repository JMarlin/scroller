#ifndef ENGINE_H
#define ENGINE_H

#include "gpu.h"
#include "module.h"

typedef struct Engine_S {
    GPU* gpu;
    GameModule module;
    void* state;
} Engine;

#endif //ENGINE_H