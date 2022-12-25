#include "module.h"
#include "event.h"
#include "gpu.h"
#include <dlfcn.h>
#include <stdio.h>
#include <emscripten.h>

void call_fp(void (*f)(GameModule), void (*init)(GPU*), void (*event)(GPU*, Event*), void (*render)(GPU*)) {
    GameModule module = {
        .init = init,
        .event = event,
        .render = render
    };

    f(module);
}

void loadModule(char* moduleName, ModuleInterfaceCallback onComplete) {
    void* handle = dlopen(moduleName, RTLD_NOW);
    printf("handle: %08X\n", (uint32_t)handle);
    if(!handle) {
        printf("dlerror: %s\n", dlerror());
        return;
    }
    void (*init)(GPU*) = dlsym(handle, "Init");
    void (*event)(GPU*, Event*) = dlsym(handle, "HandleEvent");
    void (*render)(GPU*) = dlsym(handle, "Render");

    call_fp(onComplete, init, event, render);
}

/*
void loadModule(char* moduleName, ModuleInterfaceCallback onComplete) {
    EM_ASM_((
        Module.call_fp = Module.call_fp || Module.cwrap('call_fp', 'void', ['number', 'number', 'number', 'number']);

        const moduleNamePtr = $0;
        const moduleNamePath = UTF8ToString(moduleNamePtr);
        var newEnv = {
            "EngineSetSprite": Module._EngineSetSprite,
            "EngineGetSprite": Module._EngineGetSprite
        };
        Object.assign(newEnv, asmLibraryArg);
        const importContext = {
            env: newEnv,
            wasi_snapshot_preview1: asmLibraryArg
        };

        fetch(moduleNamePath + ".wasm").then(response =>
            response.arrayBuffer()
        ).then(bytes =>
            WebAssembly.instantiate(bytes, importContext)
        ).then(results => {
            const init = addFunction(results.instance.exports.Init);
            const event = addFunction(results.instance.exports.HandleEvent);
            const render = addFunction(results.instance.exports.Render);
            debugger;
            results.instance.exports['__wasm_call_ctors']();
            results.instance.exports.main();
            Module.call_fp($1, init, event, render);
        });
    ), moduleName, onComplete);
}
*/