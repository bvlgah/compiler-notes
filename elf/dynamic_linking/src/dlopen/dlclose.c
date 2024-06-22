#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  const char *LibPath = "$ORIGIN/libbar.so";
  void *Handle = dlopen(LibPath, RTLD_GLOBAL | RTLD_LAZY);
  if (!Handle) {
    fprintf(stderr, "error: failed to load '%s': %s\n", LibPath, dlerror());
    return EXIT_FAILURE;
  }

  const char *SymbolName = "bar";
  void *Symbol = dlsym(RTLD_DEFAULT, SymbolName);
  if (!Symbol) {
    fprintf(stderr, "error: failed to find symbol '%s': %s\n",
            SymbolName, dlerror());
    return EXIT_FAILURE;
  }

  if (dlclose(Handle)) {
    fprintf(stderr, "error: failed to unload the library: %s\n", dlerror());
    return EXIT_FAILURE;
  } else {
    printf("trying to unload library '%s'\n", LibPath);
  }

  if (!(Symbol = dlsym(RTLD_DEFAULT, SymbolName))) {
    fprintf(stderr, "error: failed to find symbol '%s': %s\n",
            SymbolName, dlerror());
    return EXIT_FAILURE;
  } else {
    printf("symbol '%s' still exists even though dlclose was invoked\n",
           SymbolName);
  }

  return EXIT_SUCCESS;
}
