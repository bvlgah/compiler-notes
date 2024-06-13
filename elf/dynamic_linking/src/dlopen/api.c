#include "api.h"

#include <dlfcn.h>
#include <stdio.h>

void greet(void) {
  puts("Hello!");
}

void *dlPromoteToGlobal(const char *LibPath) {
  return dlopen(LibPath, RTLD_LAZY | RTLD_GLOBAL | RTLD_NOLOAD);
}
