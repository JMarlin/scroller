#ifndef MODULE_H
#define MODULE_H

#include "event.h"

typedef struct GameModule_S {
    void (*init)(struct Engine_S*);
    void (*event)(struct Engine_S*, Event*);
    void (*render)(struct Engine_S*);
} GameModule;

typedef void (*ModuleInterfaceCallback)(GameModule);

void loadModule(char* moduleName, ModuleInterfaceCallback onComplete);

#endif //MODULE_H