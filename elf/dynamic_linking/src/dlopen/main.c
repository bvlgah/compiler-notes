#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef NDEBUG
#define LOG_DEBUG(...)
#else
#define LOG_DEBUG(...)                                              \
  do {                                                              \
    fprintf(stderr, "DEBUG: ");                                     \
    fprintf(stderr, __VA_ARGS__);                                   \
  } while (0)
#endif

typedef void (*GreetFuncType)(void);

typedef void *(*PromoteFuncType)(const char *);

static char *getPathFromCwd(const char *FileName) {
  if (FileName[0] == '/')
    return strdup(FileName);

  char Buf[4000];
  size_t NameLength = strlen(FileName);
  if (NameLength + 1 > sizeof(Buf)) {
    errno = ERANGE;
    return NULL;
  }

  if (!getcwd(Buf, sizeof(Buf) - 1 - NameLength)) {
    errno = ERANGE;
    return NULL;
  }

  size_t CwdLength = strlen(Buf);
  assert(CwdLength > 0 && "path of the current working directory is empty");
  char *Dst = Buf + CwdLength;
  if (*Dst == '/')
    ++Dst; // Dst points to '\0'
  else
    *(Dst++) = '/';
  memcpy(Dst, FileName, NameLength);
  *(Dst + NameLength) = '\0';
  return strdup(Buf);
}

static void *load(Lmid_t Nsid, const char *LibPath, int Mode) {
  void *Handle = dlmopen(Nsid, LibPath, Mode);
  if (!Handle) {
    fprintf(stderr, "failed to load %s: %s\n", LibPath, dlerror());
    exit(EXIT_FAILURE);
  }
  return Handle;
}

static void *lookupSymbol(void *Handle, const char *Name) {
  dlerror();
  void *Symbol = dlsym(Handle, Name);
  char *DlError = dlerror();
  if (DlError) {
    fprintf(stderr, "failed to find the symbol '%s': %s\n", Name, DlError);
    exit(EXIT_FAILURE);
  }
  return Symbol;
}

Lmid_t getNamespace(void *Handle) {
  Lmid_t Namespace;
  dlerror();
  if (dlinfo(Handle, RTLD_DI_LMID, &Namespace)) {
    fprintf(stderr, "failed to get the namespace of the handle %p: %s\n",
            Handle, dlerror());
    exit(EXIT_FAILURE);
  }
  return Namespace;
}

struct link_map *getFirstMap(void *Handle) {
  struct link_map *FirstMap;
  dlerror();
  if (dlinfo(Handle, RTLD_DI_LINKMAP, &FirstMap)) {
    fprintf(stderr, "failed to get the link map from the handle %p: %s\n",
            Handle, dlerror());
    exit(EXIT_SUCCESS);
  }

  struct link_map *Prev;
  while ((Prev = FirstMap->l_prev))
    FirstMap = Prev;
  return FirstMap;
}

static void printNamespace(void *Handle) {
  printf("Handle %p belongs to the namespace %ld, "
         "which has the following shared libraries:\n",
         Handle, (long) getNamespace(Handle));
  struct link_map *FirstMap = getFirstMap(Handle);

  while (FirstMap) {
    printf("    Shared library '%s' is loaded at %p\n",
           FirstMap->l_name, (void *) FirstMap->l_addr);
    FirstMap = FirstMap->l_next;
  }
}

int main(void) {
  void *MainHandle = load(LM_ID_BASE, NULL, RTLD_LAZY | RTLD_NOLOAD);

  const char *ApiLibName = "libapi.so";
  char *ApiLibPath = getPathFromCwd(ApiLibName);
  int Error = errno;
  if (!ApiLibPath) {
    fprintf(stderr,
            "failed to get the absolute path of '%s' under the CWD: %s\n",
            ApiLibName, strerror(Error));
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG("The path of '%s' is '%s'\n", ApiLibName, ApiLibPath);

  void *BaseApiHandle = load(LM_ID_BASE, ApiLibPath, RTLD_LAZY | RTLD_GLOBAL);
  void *IsoApiHandle = load(LM_ID_NEWLM, ApiLibPath, RTLD_LAZY);

  const char *GreetFuncName = "greet";
  GreetFuncType IsoGreet =
      (GreetFuncType) lookupSymbol(IsoApiHandle, GreetFuncName);
  GreetFuncType BaseGreet =
      (GreetFuncType) lookupSymbol(BaseApiHandle, GreetFuncName);

  LOG_DEBUG("IsoApiHandle is at %p\n", IsoApiHandle);
  LOG_DEBUG("BaseApiHandle is at %p\n", BaseApiHandle);

  printf("Function '%s' in the initial namespace is at %p\n",
         GreetFuncName, BaseGreet);
  printf("Function '%s' in the new namespace is at %p\n",
         GreetFuncName, IsoGreet);

  printNamespace(BaseApiHandle);
  printNamespace(IsoApiHandle);

  const char *PromoteFuncName = "dlPromoteToGlobal";
  PromoteFuncType PromoteToGlobal = (PromoteFuncType) lookupSymbol(
      IsoApiHandle, PromoteFuncName);
  void *AnotherApiHandle = PromoteToGlobal(ApiLibPath);
  if (!AnotherApiHandle) {
    fprintf(stderr, "failedl to promote library '%s' to the global scope: %s\n",
            ApiLibPath, dlerror());
  }

  free(ApiLibPath);
  return EXIT_SUCCESS;
}
