#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error_handling.h"
#include "ptrace_helper.h"

static void doChild(unsigned sleepSeconds) {
  sleep(sleepSeconds);
  exit(EXIT_SUCCESS);
}

static void doParent(pid_t child) {
  PTRACE_WRAPPER(PTRACE_ATTACH, child, /*addr*/NULL, /*data*/NULL);

  siginfo_t info;
  id_t pid = (id_t) child;
  waitid_wrapper(P_PID, pid, &info, WEXITED | WSTOPPED);
  RUNTIME_ASSERT(info.si_code == CLD_TRAPPED, "si_code is not CLD_TRAPPED");
  printf("child process %ld has been attached\n", (long) pid);

  siginfo_t childSigInfo;
  PTRACE_WRAPPER(PTRACE_GETSIGINFO, child, /*addr*/NULL, &childSigInfo);
  psiginfo(&childSigInfo, "signal info of the process");

  PTRACE_WRAPPER(PTRACE_CONT, child, /*addr*/NULL, /*data*/0);
  waitid_wrapper(P_PID, pid, &info, WEXITED | WSTOPPED);
  RUNTIME_ASSERT(info.si_code == CLD_EXITED,
      "child process has stopped, but exit is expected");
  RUNTIME_ASSERT(info.si_status == EXIT_SUCCESS,
      "child process didn't exit successfully");
  puts("child process exited successfully");
}

int main(void) {
  pid_t pid = fork();
  if (pid == -1)
    PANIC_WITH_ERRNO("failed to fork", errno);
  else if (pid == 0)
    doChild(10);
  else
    doParent(pid);

  return EXIT_SUCCESS;
}
