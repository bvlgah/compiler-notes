#include "api.h"
#include "util.h"

#include <dlfcn.h>
#include <stdio.h>

bool hasSymbolDefault(const char *Symbol) {
  return lookupSymbol(RTLD_DEFAULT, Symbol);
}

bool printMainModule(void) {
  void *Main = loadLibrary(NULL, RTLD_LOCAL | RTLD_LAZY | RTLD_NOLOAD);
  if (!Main)
    return true;

  if (printDSOHandle(stdout, Main)) {
    unloadLibrary(Main);
    return true;
  }
  return unloadLibrary(Main);
}
