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
  CheckFuncType CheckDefault = (CheckFuncType) lookupSymbol(LibFoo,
      "checkSymbolDefault");
  CheckFuncType CheckMain = (CheckFuncType) lookupSymbol(LibFoo,
      "checkSymbolMain");
  if (!PrintMain || !CheckDefault || !CheckMain) goto onError;

  const char *SymbolName = "bar";
  if (PrintMain()) goto onError;
  CheckDefault(SymbolName);
  CheckMain(SymbolName);

  unloadLibrary(LibBar);
  if (PrintMain()) goto onError;
  CheckDefault(SymbolName);
  CheckMain(SymbolName);

  unloadLibrary(LibFoo);
  return EXIT_SUCCESS;

onError:
  if (LibBar)
    unloadLibrary(LibBar);
  if (LibFoo)
    unloadLibrary(LibFoo);
  return EXIT_FAILURE;
}
