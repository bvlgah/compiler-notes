#include "api.h"
#include "util.h"

#include <dlfcn.h>
#include <stdio.h>

bool checkSymbolDefault(const char *Symbol) {
  LOG_DEBUG("trying to look up symbol '%s' with RTLD_DEFAULT\n", Symbol);

  if (lookupSymbol(RTLD_DEFAULT, Symbol)) {
    LOG_DEBUG("symbol '%s' exists\n", Symbol);
    return false;
  } else {
    LOG_DEBUG("symbol '%s' does not exist\n", Symbol);
    return true;
  }
}

bool printMainModule(void) {
  LOG_DEBUG("trying to print main module\n");

  void *Main = loadLibrary(NULL, RTLD_LOCAL | RTLD_LAZY | RTLD_NOLOAD);
  if (!Main)
    return true;

  if (printDSOHandle(stdout, Main)) {
    unloadLibrary(Main);
    return true;
  }
  return unloadLibrary(Main);
}

bool checkSymbolMain(const char *Symbol) {
  LOG_DEBUG("trying to look up symbol '%s' in main module\n", Symbol);

  void *Main = loadLibrary(NULL, RTLD_LOCAL | RTLD_LAZY | RTLD_NOLOAD);
  if (!Main)
    return true;

  if (lookupSymbol(Main, Symbol)) {
    LOG_DEBUG("symbol '%s' exists in main module\n", Symbol);
    return false;
  } else {
    LOG_DEBUG("symbol '%s' does not exist in main module\n", Symbol);
    return true;
  }
}
