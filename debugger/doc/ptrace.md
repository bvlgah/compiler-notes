# `ptrace` FAQ

## Is a child processes of the tracee traced by the tracer?

A tracer establishes a tracing relationship with another process (a.k.a. tracee)
either by attaching to it through the system call `ptrace` and the operation
`PTRACE_ATTACH`, or the to-be-traced process' calling `ptrace` with the
operation `PTRACE_ME` if it is a child process of the tracer. Does a child
process (effectively a grandchild process) of the tracee process become a tracee
of its grandparent process (the initial tracer) if the tracee process forks?

The short answer is no. The grandchild process does not automatically become a
tracee of the grandparent process if the `ptrace` option `PTRACE_O_TRACEFORK` is
not set. If the option can be enabled via the operation `PTRACE_SETOPTIONS`,
the tracer will first get a notification one that the child process is stopped
due to calling `fork()`, allowing the pid of the grandchild process to be
retrieved with the operation `PTRACE_GETEVENTMSG`. After the child process
resumes, the tracer gets the second notification that the grandchild process is
stopped, ensuring the ordering of the child process being stopped first. See
an example in [debugger/ptrace/fork](/debugger/ptrace/fork.c). Other trace
events are summarized as follows:

| `ptrace` Options | Event | Description |
| ---------------- | ----- | ----------- |
| `PTRACE_O_TRACEFORK` | `fork()` | At first, the process calling `fork()` is
stopped. After it continues, the Linux kernel stops the created new process. |
| `PTRACE_O_TRACECLONE` | `clone()` | It is similar to `PTRACE_O_TRACEFORK` |
| `PTRACE_O_TRACEEXIT` | `_exit()` | The calling process is stopped while it has
not been finished exiting, permitting the tracer to examine the context. But the
tracer cannot prevent the exit. |
| `PTRACE_O_TRACEEXEC` | `execve()` or `execveat()` | The process is stopped by
the kernel after it has loaded a new program. |
| `PTRACE_O_TRACEVFORK` | `vfork()` | The calling process is stopped before the
newly created process runs, while the tracer can inspect pid of the child
process. After the calling process resumes, it is suspended until its child
process loads a new program. |
| `PTRACE_O_TRACEVFORKDONE` | `vfork()` | The calling process is stopped as it
has comppleted the function call, implying its child process has already loaded
a new program. |
