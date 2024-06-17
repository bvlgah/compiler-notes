#pragma once

#define _GNU_SOURCE

#include <dlfcn.h>
#include <link.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef NDEBUG
#define LOG_DEBUG(...)
#else
#define LOG_DEBUG(...)                                                    \
  do {                                                                    \
    fprintf(stderr, "DEBUG %s (%s): ", __func__, __FILE__);               \
    fprintf(stderr, __VA_ARGS__);                                         \
  } while (0)
#endif

extern char *getPathFromCwd(const char *FileName);

extern bool printNamespace(FILE *Stream, void *Handle);

extern void *lookupSymbol(void *Handle, const char *Symbol);

extern bool printDSOHandle(FILE *Stream, void *Handle);

extern void *loadLibrary(const char *Path, int Flag);

extern void *loadLibraryNS(Lmid_t Nsid, const char *Path, int Flag);

extern bool unloadLibrary(void *Handle);

extern bool getNamespace(void *Handle, Lmid_t *Namespace);
