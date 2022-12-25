#ifndef MODULE_H
#define MODULE_H

#include "gpu.h"
#include "event.h"

typedef struct GameModule_S {
    void (*init)(GPU*);
    void (*event)(GPU*, Event*);
    void (*render)(GPU*);
} GameModule;

typedef void (*ModuleInterfaceCallback)(GameModule);

void loadModule(char* moduleName, ModuleInterfaceCallback onComplete);

#endif //MODULE_H