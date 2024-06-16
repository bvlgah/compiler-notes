#define _GNU_SOURCE

#include "util.h"

#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  void *LibBar = NULL;
  void *LibFoo = NULL;

  LibBar = loadLibraryNS(LM_ID_NEWLM, "$ORIGIN/libbar.so",
      RTLD_LOCAL | RTLD_LAZY);
  if (!LibBar) goto onError;

  Lmid_t NewNamespace;
  if (getNamespace(LibBar, &NewNamespace))
    goto onError;

  LibFoo = loadLibraryNS(NewNamespace, "$ORIGIN/libfoo.so",
      RTLD_LOCAL | RTLD_LAZY);
  if (!LibFoo) goto onError;

  typedef bool (*PrintMainType)(void);
  typedef bool (*CheckFuncType)(const char*);
  PrintMainType PrintMain = (PrintMainType) lookupSymbol(LibFoo,
      "printMainModule");
  CheckFuncType Check = (CheckFuncType) lookupSymbol(LibFoo,
      "hasSymbolDefault");
  if (!PrintMain || !Check) goto onError;

  Check("bar");
  if (PrintMain()) goto onError;
  unloadLibrary(LibBar);
  Check("bar");
  if (PrintMain()) goto onError;

  unloadLibrary(LibFoo);
  return EXIT_SUCCESS;

onError:
  if (LibBar)
    unloadLibrary(LibBar);
  if (LibFoo)
    unloadLibrary(LibFoo);
  return EXIT_FAILURE;
}
