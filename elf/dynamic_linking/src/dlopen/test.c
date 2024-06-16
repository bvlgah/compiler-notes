#include "api.h"
#include "util.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>

#define FOO_PATH            "$ORIGIN/libfoo.so"
#define BAR_PATH            "$ORIGIN/libbar.so"

#define PRE_TEST_CASE                                                     \
  bool Err = false;                                                       \
  do {                                                                    \
    printf("running test case '%s'\n", __func__);                         \
  } while (0)

#define POST_TEST_CASE                                                    \
  do {                                                                    \
    if (Err)                                                              \
      puts("failure");                                                    \
    else                                                                  \
      puts("success");                                                    \
  } while (0)                                                             \

static bool testMain() {
  PRE_TEST_CASE;
  void *Main = loadLibrary(NULL, RTLD_LOCAL | RTLD_LAZY | RTLD_NOLOAD);
  if (!Main) {
    Err = true;
    goto ret;
  }
  Err = printDSOHandle(stderr, Main);
  Err |= unloadLibrary(Main);

ret:
  POST_TEST_CASE;
  return Err;
}

static bool testFoo() {
  PRE_TEST_CASE;
  void *LibFoo = loadLibrary(FOO_PATH, RTLD_LOCAL | RTLD_LAZY);
  if (!LibFoo) {
    Err = true;
    goto ret;
  }

  typedef bool (*CheckFuncType)(const char*);

  CheckFuncType Check = (CheckFuncType) lookupSymbol(LibFoo,
      "hasSymbolDefault");
  if (!Check || !Check("runTest")) {
    Err = true;
    unloadLibrary(LibFoo);
    goto ret;
  }
  Err = unloadLibrary(LibFoo);

ret:
  POST_TEST_CASE;
  return Err;
}

static bool testBar() {
  PRE_TEST_CASE;

  void *LibBar = loadLibrary(BAR_PATH, RTLD_GLOBAL | RTLD_LAZY);
  if (!LibBar) {
    Err = true;
    goto ret;
  }
  void *Bar = lookupSymbol(RTLD_DEFAULT, "bar");
  Err = unloadLibrary(LibBar) | !Bar;

ret:
  POST_TEST_CASE;
  return Err;
}

bool runTest(void) {
  bool Err = false;

  Err |= testMain();
  Err |= testFoo();
  Err |= testBar();

  return Err;
}
