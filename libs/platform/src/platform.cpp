#include "plt/platform.h"
#include <mimalloc.h>

const char *vkd::plt::libVersion()
{
    static const char version[] = "plt-1.0";
    return version;
}




static void dllMain(){
  
}

static void dllExit(){

}






#ifdef _WIN32
#include <windows.h>


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            dllMain();
            break;
        case DLL_PROCESS_DETACH:
            dllExit(); 
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

#elif defined(__GNUC__) || defined(__clang__)

__attribute__((constructor))
static void dll_init_function() {
    dllMain();
}

__attribute__((destructor))
static void dll_fini_function() {
    dllExit();
}

#else
#error "Unsupported platform"
#endif

