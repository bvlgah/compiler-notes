#pragma once

#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "error_handling.h"

#define PTRACE_WRAPPER(op, pid, addr, data)                           \
  ptrace_wrapper(op, pid, addr, data, #op)

static inline long
ptrace_wrapper(enum __ptrace_request op, pid_t pid, void *addr, void *data,
               char *opName) {
  long ret = ptrace(op, pid, addr, data);
  if (ret == -1) {
    int err = errno;
    PANIC_WITH_ERRNO(opName, err);
  }
  return ret;
}

static inline int
waitid_wrapper(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
  int ret = waitid(idtype, id, infop, options);
  if (ret == -1) {
    int err = errno;
    PANIC_WITH_ERRNO("failed to wait for a child", err);
  }
  return ret;
}
