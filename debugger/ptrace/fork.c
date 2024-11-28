#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error_handling.h"
#include "ptrace_helper.h"

typedef struct {
  unsigned traceFork : 1;
  unsigned traceClone : 1;
  unsigned traceExit : 1;
  unsigned traceExec : 1;
  const char *newProg;
} CmdOptions;

static void *convertPtraceOptions(const CmdOptions *options) {
  size_t opt = 0;
  if (options->traceFork)
    opt |= PTRACE_O_TRACEFORK;
  if (options->traceClone)
    opt |= PTRACE_O_TRACECLONE;
  if (options->traceExit)
    opt |= PTRACE_O_TRACEEXIT;
  if (options->traceExec)
    opt |= PTRACE_O_TRACEEXEC;

  return (void *) opt;
}

static void handleTrap(siginfo_t *infop) {
  siginfo_t detailedSigInfo;
  pid_t stoppedPid = infop->si_pid;
  PTRACE_WRAPPER(PTRACE_GETSIGINFO, stoppedPid, /*addr*/NULL,
      &detailedSigInfo);

  if (detailedSigInfo.si_signo == SIGTRAP) {
    switch(detailedSigInfo.si_code) {
    case SIGTRAP | (PTRACE_EVENT_CLONE << 8):
    case SIGTRAP | (PTRACE_EVENT_FORK << 8):
      printf("process %ld is trapped due to calling `fork()` or `clone()`\n",
          (long) stoppedPid);
      unsigned long data;
      PTRACE_WRAPPER(PTRACE_GETEVENTMSG, detailedSigInfo.si_pid, /*addr*/NULL,
          &data);
      printf("pid of the new process created by "
          "`fork()` and `clone()` is %lu\n", data);
      break;
    case SIGTRAP | (PTRACE_EVENT_EXIT << 8):
      printf("process %ld is trapped due to calling `_exit()`\n",
          (long) stoppedPid);
      break;
    case SIGTRAP | (PTRACE_EVENT_EXEC << 8):
      printf(
          "process %ld is trapped due to calling `execve()` or `execveat()`\n",
          (long) stoppedPid);
      break;
    default:
      printf("process %ld is trapped by `SIGTRAP` for unknown reasons, "
          "`si_code` = 0x%x\n",
          (long) stoppedPid, detailedSigInfo.si_code);
    }
  } else if (detailedSigInfo.si_signo == SIGSTOP) {
    printf("process %ld is trapped by `SIGSTOP`\n", (long) stoppedPid);
  } else {
    printf("process %ld is trapped by `%s`\n", (long) stoppedPid,
        strsignal(detailedSigInfo.si_signo));
  }

  PTRACE_WRAPPER(PTRACE_CONT, stoppedPid, /*addr*/NULL, /*data*/NULL);
}

static void doGrandChild(const char *newProg) {
  pid_t pid = getpid();
  printf("process %ld: ready to load the program `%s`\n", (long) pid, newProg);
  execl(newProg, newProg, NULL);
  int err = errno;
  fprintf(stderr, "process %ld: failed to load the program `%s`: %s\n",
      (long) pid, newProg, strerror(err));
  exit(EXIT_FAILURE);
}

static void doChild(const char *newProg) {
  pid_t pid = getpid();

  printf("pid of child process is %ld\n", (long) pid);
  PTRACE_WRAPPER(PTRACE_TRACEME, /*pid*/0, /*addr*/NULL, /*data*/NULL);
  raise(SIGSTOP);

  pid_t grandChildPid = fork();
  if (!grandChildPid)
    doGrandChild(newProg);
  RUNTIME_ASSERT(grandChildPid != -1, "failed to fork");
}

static void doParent(pid_t pid, pid_t childPid, const CmdOptions *options) {
  siginfo_t sigInfo;
  waitid_wrapper(P_PID, (id_t) childPid, &sigInfo, WEXITED | WSTOPPED);
  // Even though the child process is stopped by raising SIGSTOP itself,
  // `si_code` should be CLD_TRAPPED because it is being traced.
  RUNTIME_ASSERT(sigInfo.si_code == CLD_TRAPPED, "child is not being trapped");

  void *traceOption = convertPtraceOptions(options);
  PTRACE_WRAPPER(PTRACE_SETOPTIONS, childPid, /*addr*/NULL, traceOption);

  PTRACE_WRAPPER(PTRACE_CONT, childPid, /*addr*/NULL, /*data*/NULL);

  int ret;
  while((ret = waitid(P_PGID, (id_t) pid, &sigInfo, WEXITED | WSTOPPED))
      != -1) {
    switch(sigInfo.si_code) {
    case CLD_TRAPPED:
      handleTrap(&sigInfo);
      break;
    case CLD_EXITED:
      printf("process %ld has exited\n", (long) sigInfo.si_pid);
      break;
    case CLD_DUMPED:
    case CLD_KILLED:
      printf("process %ld has been killed\n", (long) sigInfo.si_pid);
      break;
    case CLD_STOPPED:
      printf("process %ld has been stopped\n", (long) sigInfo.si_pid);
      break;
    case CLD_CONTINUED:
      printf("process %ld has been continued\n", (long) sigInfo.si_pid);
      break;
    default:
      __builtin_unreachable();
    }
  }

  int err = errno;
  if (err == ECHILD)
    printf("all children have finished running\n");
  else
    PANIC_WITH_ERRNO("`waitid` failed", err);
}

static void printUsageAndExit(const char *progName) {
  fprintf(stderr, "Usage: %s --new-prog <prog>"
      "[--trace-fork] [--trace-fork] [--trace-exit]\n",
      progName);
  exit(EXIT_FAILURE);
}

static void parseOptions(const char *argv[], CmdOptions *options) {
  const char *progName = *(argv++);
  options->newProg = NULL;
  options->traceFork = 0;
  options->traceClone = 0;
  options->traceExit = 0;
  options->traceExec = 0;

  for (; *argv != NULL; argv++) {
    if (!strcmp(*argv, "--trace-fork")) {
      options->traceFork = 1;
      continue;
    }
    if (!strcmp(*argv, "--trace-clone")) {
      options->traceClone = 1;
      continue;
    }
    if (!strcmp(*argv, "--trace-exit")) {
      options->traceExit = 1;
      continue;
    }
    if (!strcmp(*argv, "--trace-exec")) {
      options->traceExec = 1;
      continue;
    }
    if (!strcmp(*argv, "--new-prog")) {
      if (*(++argv) == NULL) {
        fprintf(stderr, "%s: the option `--new-prog` requires a program name\n",
            progName);
        printUsageAndExit(progName);
      }
      options->newProg = *argv;
      continue;
    }
    fprintf(stderr, "%s: unknown option `%s`\n", progName, *argv);
    printUsageAndExit(progName);
  }

  if (options->newProg == NULL) {
    fprintf(stderr, "%s: a new program to load is required\n", progName);
    printUsageAndExit(progName);
  }
}

int main(int argc, const char *argv[]) {
  pid_t selfPid;
  pid_t childPid;

  CmdOptions options;
  parseOptions(argv, &options);

  selfPid = getpid();
  childPid = fork();
  if (childPid == -1)
    PANIC_WITH_ERRNO("failed to fork", errno);
  else if (childPid == 0)
    doChild(options.newProg);
  else
    doParent(selfPid, childPid, &options);

  return EXIT_SUCCESS;
}
