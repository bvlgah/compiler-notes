#define _GNU_SOURCE

#include "util.h"

#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char **argv) {
  void *LibBar = NULL;
  void *LibFoo = NULL;
  int DefaultFlag = RTLD_LOCAL;

  if (argc > 1) {
    if (!strcmp(argv[1], "-g")) {
      DefaultFlag = RTLD_GLOBAL;
    }
    else {
      fprintf(stderr, "error: unknown option '%s'\n", argv[1]);
      return EXIT_FAILURE;
    }
  }

  LibBar = loadLibraryNS(LM_ID_NEWLM, "$ORIGIN/libbar.so",
      DefaultFlag | RTLD_LAZY);
  if (!LibBar) goto onError;

  Lmid_t NewNamespace;
  if (getNamespace(LibBar, &NewNamespace))
    goto onError;

  LibFoo = loadLibraryNS(NewNamespace, "$ORIGIN/libfoo.so",
      DefaultFlag | RTLD_LAZY);
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
