#define _GNU_SOURCE

#include "util.h"

#include <dlfcn.h>
#include <link.h>
#include <stdbool.h>
#include <stdlib.h>

typedef bool (*RunTestFuncType)(void);

#define TEST_PATH           "$ORIGIN/libdltest.so"
#define TEST_FUNC_NAME      "runTest"

#define DEFINE_NAMESPACE_TEST(NAMESPACE, NSID)                            \
                                                                          \
bool testNamespace##NAMESPACE() {                                         \
  printf("running test suite '%s'\n", __func__);                          \
  bool Status = false;                                                    \
  void *TestLib = loadLibraryNS(NSID, TEST_PATH, RTLD_LOCAL | RTLD_NOW);  \
  if (!TestLib) {                                                         \
    Status = true;                                                        \
    goto ret;                                                             \
  }                                                                       \
  RunTestFuncType RunTest = (RunTestFuncType) lookupSymbol(               \
      TestLib, TEST_FUNC_NAME);                                           \
  if (!RunTest) {                                                         \
    Status = true;                                                        \
    goto ret;                                                             \
  }                                                                       \
  Status = RunTest();                                                     \
                                                                          \
ret:                                                                      \
  if (TestLib)                                                            \
    Status |= unloadLibrary(TestLib);                                     \
  return Status;                                                          \
}

DEFINE_NAMESPACE_TEST(Base, LM_ID_BASE)

DEFINE_NAMESPACE_TEST(New, LM_ID_NEWLM)

int main(void) {
  bool Status = false;
  Status |= testNamespaceBase();
  Status |= testNamespaceNew();
  return Status ? EXIT_FAILURE : EXIT_SUCCESS;
}
