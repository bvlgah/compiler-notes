#define _GNU_SOURCE

#include "util.h"

#include <assert.h>
#include <dlfcn.h>
#include <link.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct link_map *getMap(void *Handle) {
  struct link_map *Map;
  dlerror();
  if (dlinfo(Handle, RTLD_DI_LINKMAP, &Map)) {
    char *Err = dlerror();
    fprintf(stderr, "error: failed to get link map from handle %p: %s\n",
            Handle, Err);
    Map = NULL;
  }
  return Map;
}

static struct link_map *getFirstMap(void *Handle) {
  struct link_map *FirstMap = getMap(Handle);
  if (!FirstMap)
    return NULL;

  struct link_map *Prev;
  while ((Prev = FirstMap->l_prev))
    FirstMap = Prev;
  return FirstMap;
}

bool getNamespace(void *Handle, Lmid_t *Namespace) {
  dlerror();
  if (dlinfo(Handle, RTLD_DI_LMID, Namespace)) {
    fprintf(stderr, "error: failed to get namespace of handle %p: %s\n",
            Handle, dlerror());
    return true;
  }
  return false;
}

void *lookupSymbol(void *Handle, const char *Name) {
  dlerror();
  void *Symbol = dlsym(Handle, Name);
  char *DlError = dlerror();
  if (DlError)
    fprintf(stderr, "error: failed to find symbol '%s': %s\n", Name, DlError);
  return Symbol;
}

static void printMap(FILE *Stream, struct link_map *Map) {
  fprintf(Stream, "shared library '%s' loaded at %p\n",
          Map->l_name, (void *) Map->l_addr);
}

bool printNamespace(FILE *Stream, void *Handle) {
  Lmid_t Namespace;
  if (!getNamespace(Handle, &Namespace))
    fprintf(Stream, "namespace %p has the following shared libraries:\n",
            (void *) Namespace);
  else
    return true;

  struct link_map *FirstMap = getFirstMap(Handle);
  if (!FirstMap)
    return true;

  while (FirstMap) {
    fprintf(Stream, "    ");
    printMap(Stream, FirstMap);
    FirstMap = FirstMap->l_next;
  }
  return false;
}

bool printDSOHandle(FILE *Stream, void *Handle) {
  struct link_map *Map = getMap(Handle);
  if (!Map)
    return true;
  fprintf(Stream, "handle %p represents ", Handle);
  printMap(Stream, Map);
  return false;
}

// Don't free the return value.
static const char *getNamespaceDesc(Lmid_t Nsid) {
  static char Buf[30];
  switch (Nsid) {
  case LM_ID_BASE:
    return "LM_ID_BASE";
  case LM_ID_NEWLM:
    return "LM_ID_NEWLM";
  default:
    snprintf(Buf, sizeof(Buf), "%p", (void *) Nsid);
    return Buf;
  }
}

static char *appendString(char *Dst, size_t *Size, const char *Str) {
  int Ret = snprintf(Dst, *Size, "%s", Str);
  if (Ret <= 0) {
    return Dst;
  }
  else {
    *Size -= (size_t) Ret;
    return Dst + (size_t) Ret;
  }
}

// Don't free the return value.
static const char *getFlagDesc(int Flag) {
  static char Buf[512];
  char *Cur = Buf;
  size_t Size = sizeof(Buf);
  bool First = true;

  Cur = appendString(Cur, &Size, "(");

#define OUTPUT_RTLD_FLAG(FLAG)                                            \
  do {                                                                    \
    if (Flag & FLAG) {                                                    \
      if (!First) Cur = appendString(Cur, &Size, "|");                    \
      Cur = appendString(Cur, &Size, #FLAG);                              \
      First = false;                                                      \
    }                                                                     \
  } while (0)

  OUTPUT_RTLD_FLAG(RTLD_GLOBAL);
  OUTPUT_RTLD_FLAG(RTLD_LOCAL);
  OUTPUT_RTLD_FLAG(RTLD_LAZY);
  OUTPUT_RTLD_FLAG(RTLD_NOW);
  OUTPUT_RTLD_FLAG(RTLD_NOLOAD);
  OUTPUT_RTLD_FLAG(RTLD_NODELETE);

#undef OUTPUT_RTLD_FLAG

  Cur = appendString(Cur, &Size, ")");

  return Buf;
}

void *loadLibrary(const char *Path, int Flag) {
  LOG_DEBUG("trying to load library from path '%s' with %s\n",
            Path, getFlagDesc(Flag));
  void *Handle = dlopen(Path, Flag);

  if (!Handle)
    fprintf(stderr, "error: failed to load shared library '%s': %s\n",
            Path, dlerror());
  return Handle;
}

void *loadLibraryNS(Lmid_t Nsid, const char *Path, int Flag) {
  LOG_DEBUG("trying to load library from path '%s' with %s into namespace %s\n",
            Path, getFlagDesc(Flag), getNamespaceDesc(Nsid));
  void *Handle = dlmopen(Nsid, Path, Flag);

  if (Handle)
    return Handle;

  fprintf(stderr, "error: failed to load shared library '%s' "
          "into namespace %s: %s\n", Path, getNamespaceDesc(Nsid), dlerror());
  return NULL;
}

bool unloadLibrary(void *Handle) {
  struct link_map *Map = getMap(Handle);
  LOG_DEBUG("trying to unload library '%s'\n", Map->l_name);
  int Error;
  if ((Error = dlclose(Handle)))
    fprintf(stderr, "error: failed to unload shared library: %s\n", dlerror());
  return (bool) Error;
}
