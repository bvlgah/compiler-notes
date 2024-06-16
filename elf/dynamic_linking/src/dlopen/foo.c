#include "api.h"
#include "util.h"

#include <dlfcn.h>

bool hasSymbolDefault(const char *Symbol) {
  return lookupSymbol(RTLD_DEFAULT, Symbol);
}
