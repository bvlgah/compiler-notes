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

static bool getNamespace(void *Handle, Lmid_t *Namespace) {
  dlerror();
  if (dlinfo(Handle, RTLD_DI_LMID, Namespace)) {
    fprintf(stderr, "error: failed to get namespace of handle %p: %s\n",
            Handle, dlerror());
    return false;
  }
  return true;
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
  fprintf(Stream, "shared library '%s' is loaded at %p\n",
          Map->l_name, (void *) Map->l_addr);
}

bool printNamespace(FILE *Stream, void *Handle) {
  Lmid_t Namespace;
  if (getNamespace(Handle, &Namespace))
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

void *loadLibrary(const char *Path, int Flag) {
  void *Handle = dlopen(Path, Flag);

  if (!Handle)
    fprintf(stderr, "error: failed to load shared library '%s': %s\n",
            Path, dlerror());
  return Handle;
}

void *loadLibraryNS(Lmid_t Nsid, const char *Path, int Flag) {
  void *Handle = dlmopen(Nsid, Path, Flag);

  if (Handle)
    return Handle;

  switch (Nsid) {
  case LM_ID_BASE:
    fprintf(stderr, "error: failed to load shared library '%s'"
            "into base namespace: %s\n", Path, dlerror());
    break;
  case LM_ID_NEWLM:
    fprintf(stderr, "error: failed to load shared library '%s'"
            "into a new namespace: %s\n", Path, dlerror());
    break;
  default:
    fprintf(stderr, "error: failed to load shared library '%s'"
            "into namespace %p: %s\n", Path, (void *) Nsid, dlerror());
  }
  return NULL;
}

bool unloadLibrary(void *Handle) {
  int Error;
  if ((Error = dlclose(Handle)))
    fprintf(stderr, "error: failed to unload shared library: %s\n", dlerror());
  return (bool) Error;
}
