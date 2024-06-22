#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  const char *LibPath = "$ORIGIN/libfoo.so";
  void *Handle = dlopen(LibPath, RTLD_LOCAL | RTLD_LAZY);
  if (!Handle) {
    fprintf(stderr, "error: failed to load %s: %s\n", LibPath, dlerror());
    return EXIT_FAILURE;
  }

  const char *SymbolName = "getNamespace";
  void *Symbol = dlsym(RTLD_DEFAULT, SymbolName);
  if (!Symbol) {
    printf("symbol '%s' is not found\n", SymbolName);
  } else {
    fprintf(stderr, "symbol '%s' is found unexpectedly\n", SymbolName);
    return EXIT_FAILURE;
  }

  Handle = dlopen(LibPath, RTLD_GLOBAL | RTLD_LAZY | RTLD_NOLOAD);
  if (!Handle) {
    fprintf(stderr, "error: failed to load %s: %s\n", LibPath, dlerror());
    return EXIT_FAILURE;
  }
  Symbol = dlsym(RTLD_DEFAULT, SymbolName);
  if (Symbol) {
    printf("symbol '%s' is found after promotion\n", SymbolName);
  } else {
    fprintf(stderr, "error: failed to find symbol '%s': %s\n",
            SymbolName, dlerror());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
