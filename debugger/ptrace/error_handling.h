#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PANIC_WITH_ERRNO(msg, errnum)                                 \
  do {                                                                \
    fprintf(stderr, "%s: %s\n", msg, strerror(errnum));               \
    exit(EXIT_FAILURE);                                               \
  } while (0)

#define PANIC(msg)                                                    \
  do {                                                                \
    fprintf(stderr, "%s\n", msg);                                     \
    exit(EXIT_FAILURE);                                               \
  } while (0)

#define RUNTIME_ASSERT(expression, errMsg)                            \
  do {                                                                \
    if (!(expression))                                                \
      PANIC(errMsg);                                                  \
  } while(0)
