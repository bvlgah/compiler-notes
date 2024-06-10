# How a Program is Loaded: from execve to main

Please note: the analyses in this article are primarily based on the x86-64 port
of Linux and glibc, focusing exclusively on dynamically linked executables.

## 1. Introduction

Linux `execve` and `execveat` are two syscalls used to reload a new program
image for the current process. A family of functions built upon the syscall
`execve` are provided by libc, such as `execvp` (for a complete list, refer to
[Linux manual page][linux man exec] [2]). Below is an example of using `execvp`
from Wenbo Shen's [blog][wenbo shen execve] [1]:

```c
int main(int argc, char *argv[]) {
  char * ls_args[4] = { "ls", "-l", NULL } ;
  pid_t child_pid;
  int status;

  if (child_pid == 0){
    child_pid = fork();
    printf("In child\n");
  } else if (child_pid > 0){
    execvp( ls_args[0], ls_args );
    wait(&status);
  }

  printf("In parent: finished\n");
  return 0; //return success
}
```

One rationale of using two system calls to implement process spawning, rather
than just one, is that it allows certain works to be performed during the two
calls. This design facilitates common and important feasures like I/O
redirection (for a potential implementation, see [StackOverflow][stackoverflow
io redirection] [3]).

This article will explore how a dynamically linked binary is loaded—a topic not
extensively covered by Wenbo's blog—and discuss some of the initialization work
performed by the dynamic linker after control is returned from the kernel to
user space.

## 2. Linux execve

Note: the following analysis of Linux's source code is based on [version 6.10](
https://stackoverflow.com/questions/36588387/what-happens-to-threads-when-exec-is-called),
which is under development as of the writing of this article.

Let us first examine the Linux system calls [execve](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L2143-L2149)
and [execveat](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L2151-L2160)
(the latter performs a similar function). Both are handled by
[do\_execveat\_common](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1922-L2005)
(through [do\_execve](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L2066-L2073)
and [do\_execveat](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L2075-L2084),
respectively). A struct [linux\_binprm](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/include/linux/binfmts.h#L18-L65)
is employed to represent the binary being loaded. Notably, Linux supports
various binary program formats, with ELF being the primary focus of this
article. Within `do_execveat_common`, an instace of `linux_binprm` is created
and initialized by [alloc\_bprm](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1554-L1606),
and it is subsequently passed to [bprm\_execve](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1867-L1920)
and then [exec\_binprm](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1822-L1865).

```c
static int do_execveat_common(int fd, struct filename *filename,
            struct user_arg_ptr argv,
            struct user_arg_ptr envp,
            int flags)
{
  struct linux_binprm *bprm;
  int retval;
...
  bprm = alloc_bprm(fd, filename, flags);
...
  retval = bprm_execve(bprm);
out_free:
  free_bprm(bprm);

out_ret:
  putname(filename);
  return retval;
}
```

[search\_binary\_handler](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1775-L1819)
is invoked by `exec_binprm` to search a list for an appropriate binary loader,
which is appended to the list by [\_\_register\_binfmt](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L88-L94)
(or [register\_binfmt](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/include/linux/binfmts.h#L106-L109)).
In the case of loading an ELF executable, it is the one registered by
[init\_elf\_binfmt](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L2104C19-L2108).

```c
static int exec_binprm(struct linux_binprm *bprm)
{
...
  for (depth = 0;; depth++) {
    if (depth > 5)
      return -ELOOP;

    ret = search_binary_handler(bprm);
    if (ret < 0)
      return ret;
    if (!bprm->interpreter)
      break;
  }
...
}
```

```
static struct linux_binfmt elf_format = {
  .module= THIS_MODULE,
  .load_binary= load_elf_binary,
  .load_shlib= load_elf_library,
#ifdef CONFIG_COREDUMP
  .core_dump= elf_core_dump,
  .min_coredump= ELF_EXEC_PAGESIZE,
#endif
};
```

The function [load\_elf\_binary](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L819-L1317)
is capable of loading both statically and dynamically linked executables. There
are two types of dynamic executables: PIEs (with an interpreter) and loaders (
without an interpreter, typically ELF interpreters). To load a PIE, both the
executable and the interpreter (see [load\_elf\_interp](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L631-L707))
have their ELF program headers examined (see [load\_elf\_phdrs](
https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L506-L539)
and [elf\_read](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L466-L475)),
and their loadable segments are mapped into memory accordingly (see
[elf\_load](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L400-L446)
and [elf\_map](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L356-L393)).
While the kernel initiates the loading process, much of the work remains for the
the dynamic linker in user space. Furthermore, in
[create\_elf\_tables][create elf tables], the kernel initializes `argc`, `argv`
and `env` needed by the `main` function of C programs, as well as an auxiliary
array containing ELF-related information (e.g., where the dynamic linker is
loaded) on the user stack. Their locations on the stack are precisely specified
when user space is switched to upon a successful `execve`. On x86-64, their
layout is defined by [ELF x86-64-ABI psABI][elf amd64 psabi] [5]:

| Purpose | Start Address | Length |
| ------- | ------------- | ------ |
| Unspecified | High Addresses | |
| Information block, including argu- ment strings, environment strings, auxiliary information ... | | varies |
| Unspecified | | |
| Null auxiliary vector entry | | 1 eightbyte |
| Auxiliary vector entries ... | | 2 eightbytes each |
| 0 | | eightbyte |
| Environment pointers ... | | 1 eightbyte each |
| 0 | `8+8*argc+%rsp` | eightbyte |
| Argument pointers | `8+%rsp` | argc eightbytes |
| Argument count | `%rsp` | eightbyte |
| Undefined | Low Addresses | |

This layout is utilized by the dynamic linker. Interestingly,
it appears that the same layout is adopted on other hardware architectures
beyond x86-64, as noted in some architectures' ABI specifications (e.g.,
[zSeries ELF Application Binary Interface Supplement, Process initialization][
zeries elf process initialization] [6]). Although this layout is not specified
in ABI specifications of all architectures by Linux (e.g., I have not found
it in the psABI specifications of RISCV and AArch64), it is likely that the
layout applies universally because the code for handling the layout in user
space is [architecture-independent](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L312-L347). With regard to handling `argv`/`envp`,
[oakad][stackoverflow elf argc argv envp] [4] has summarized it as a two-stage
process:

- The kernel saves the actual characters on the new process' stack regardless
of the binary format.

- The ELF loader stores pointers to these strings on the stack according to
the layout mentioned above.

If `load_elf_binary` returns successful, the kernel's porting of the ELF loading
work is nearly completed. Following a successful `do_execveat_common`, the
followings are true once control is returned back to user space for dynmiac PIE
executables:

- The executable, the interpreter and vDSO (if enabled) are mapped into memory.

- The stack pointer (`sp`) points to the start of an region of memory storing
`argc`, `argv`, `env` and `auxv`.

- The process begins at the entry point of the interpreter, as indicated in
[load\_elf\_binary](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L1200).

Refer to the [Appendix](#linux-execv) for the call tree of `execv` and links to
related source code.

## 3. Glibc Dynamic Linker

Glibc is not only the most predominant implementation of the C standard and
POSIX library on Linux distributions, but it also serves as a widely-used
dynamic linker for Linux users. The dynamic linker component of glibc, known as
glibc rtld, is distributed as a separate shared library, typically located at
`/lib64/ld-linux-x86-64.so.2` on an x86-64 Linux OS. This section will explore
the dynamic linker by providing an overview of its functionality and detailing
several of its essential aspects.

### 3.1 Overview

Before any function in the executable is run, the operations performed by glibc
rtld can be summarized as follows:

1. In `_dl_start`, the rtld determines the runtime addresses of several ELF
sections for later use, and performs relocation for itself to enable accesses to
data and functions via the Global Offset Table (GOT). At the end of `_dl_start`,
a tail call is made to `_dl_start_final`.

2. `_dl_start_final` (which can be regarded as the second half of
`_dl_start`) delegates OS-dependent initialization tasks to `_dl_sysdep_start`.
On Linux, this initialization includes parsing command-line arguments,
environment variables and the ELF auxiliary vector.

3. Following `_dl_start_final` is `dl_main`. Initially, the environment
variables (prefixed with `LD_`) that control the rtld's behaviors are processed.
And an internal representation of the executable (noted by the variable
`main_map` in the function) is created and populated with both static
(precreated) and runtime information, such as the name specified by `DT_SONAME`
and the location of the symbol hash table. If the vDSO is present, a similar
representation is initialized for it and searched for certain functions to be
utilized later. Subsequently, dependency libraries, either specified by
`DT_NEEDED` in the executable (direct dependencies) or referenced transitively
by `DT_NEEDED` in any newly enrolled dependency (transitive dependencies), are
loaded. Furthermore, optional libraries—designated by the environment variable
`LD_PRELOAD`, the command line argument `--preload` (if the rtld is run as the
executable) and the file `/etc/ld.so.preload`—are loaded before the executable's
dependencies, known as preloaded libraries. Finally, references to global data
and functions in preloaded and dependency libraries are resolved through
relocation, typically with version information utilized (refer to [my other
article about symbol versioning](/elf/dynamic_linking/symbol_versioning.md)).
Additionally, the rtld itself is an executable primarily designed to load other
executables for testing and diagnoses, which takes the first command-line
argument as the path of the executable to be loaded.

4. After the rtld's `_start` is completed, `__dl_start_user` is invoked to
perform a few final steps before jumping to the entry point of the executable.
According to x86-64 psABI [5], if the entry point of the executable is reached,
`%rdx` will contain a function pointer which the executable should register with
`atexit`. In glibc, `%rdx` is set to the runtime address of [\_dl\_fini](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-fini.c#L24-L145),
as demonstrated by the code in [\_dl\_start\_user](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L170).
It is worth noting that the C/C++ `main` function is different from the entry
point of an ELF executable. Some preparatory work must be done before reaching
`main` function, which will be explored in future updates.

Unfortunately, Thread Local Storage (TLS) remains one of the unexplored aspects
of glibc rtld, expected to be discussed in forthcoming updates to this article.

### 3.2 Details of Glibc rtld

#### 3.2.1 Determing Load Addresses of Executable, rtld and vDSO

How are the load addresses of these three shared libraries, which are mapped
into memory by the Linux kernel, determined by the rtld?

Insight can be gained from glibc's [elf\_machine\_load\_address][x86-64 elf
machine load address]:

```c
/* Return the run-time load address of the shared object.  */
static inline ElfW(Addr) __attribute__ ((unused))
elf_machine_load_address (void)
{
  extern const ElfW(Ehdr) __ehdr_start attribute_hidden;
  return (ElfW(Addr)) &__ehdr_start;

}
```

`__ehdr_start` is a reserved symbol defined by linkers like GNU ld and LLVM lld,
set to the address of the ELF header. For LLVM lld, its implementation is found
in [lld/ELF/Writer.cpp](https://github.com/llvm/llvm-project/blob/6c973036818f926c65ddc9b40578917e5f2240cb/lld/ELF/Writer.cpp#L222).
The runtime address of the ELF header is the same as the address where the rtld
is loaded, applicable to any shared library. This reserved symbol is internal,
thus it is only permissble for it to be accessed within the defining component
(a dynamic executable or library). Based on previous experience, the accesses
can be implemented with PC-relative addressing.

Similarly, the runtime address of the section `.dynamic`, which contains vital
dynamic linking information, is determined by the reserved symbol `_DYNAMIC`.
For further details, see glibc's [elf\_machine\_dynamic][elf machine dynamic]
and lld's [Writer\<ELFT\>::finalizeSections](https://github.com/llvm/llvm-project/blob/6c973036818f926c65ddc9b40578917e5f2240cb/lld/ELF/Writer.cpp#L1690-L1699).

An executable is typically loaded by the Linux kernel, either when run from the
shell or via the `execv` system call. Alternatively, the executable can be
executed by launching the dynamic linker directly with the executable's path. If
the kernel loads the executable, it must furnish the dynamic linker with details
necessary to ascertain the runtime address where the executable is loaded. This
information is conveyed through an auxiliary array, established by 
[create\_elf\_tables][create elf tables] in Linux's [fs/binfmt\_elf.c][linux
binfmt elf]. The array contains the following (non-exhaustive) details:

| Auxv Type | Value |
| --------- | ----- |
| AT\_PHDR | Runtime address where the executable's program headers begin |
| AT\_PHENT | Size of one program header entry |
| AT\_PHNUM | Total number of program header entries |
| AT\_ENTRY | Entry point address of the executable at runtime |
| AT\_SYSINFO\_EHDR | Runtime address where vDSO is loaded |

The load address of vDSO is readily available, whereas the executable's address
requires computation—subtracting the static starting address of the program
headers from `AT_PHDR`, as carried out in
[rtld\_setup\_main\_map][rtld setup main map].

#### 3.2.2 `DT_RPATH` and `DT_RUNPATH`

Through `_dl_map_object`, we observe how glibc manages `DT_RPATH` (now
deprecated in favor of `DT_RUNPATH`). `DT_RPATH` designates a colon-separated
list of directories where the dynamic linker searches for components by relative
paths. Suppose a shared object `A` relies on another shared object `B`.
Initially, `_dl_map_object` will peruse directories listed in `A`'s `DT_RPATH`
for `B`. If `B` remians unfound, directories from `DT_RPATH` of `A`'s loader
(`A`'s dependent) are checked, continuing until no additional loaders remain.
If B still eludes discovery, and the executable itself has yet to be examined,
directories specified in the executable's `DT_RPATH` are then searched. Glibc's
treatment of `DT_RPATH` in [DT\_RPATH in\_dl\_map\_object](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-load.c#L2029-L2060)
elaborates on this process, and [the same function](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-load.c#L2078-L2091)
also handles the environment variable `LD_LIBRARY_PATH` and ELF `DT_RUNPATH`,
employing only `DT_RUNPATH` of the immediate loader, which is `A` in the
previous example, for searches.

The following code snippet from glibc's
[elf\_get\_dynamic\_info][elf get dynamic info] highlights how `DT_RPATH` is
suppressed by `DT_RUNPATH` if both are present:

```c
static inline void __attribute__ ((unused, always_inline))
elf_get_dynamic_info (struct link_map *l, bool bootstrap,
          bool static_pie_bootstrap)
{
...
    if (info[DT_RUNPATH] != NULL)
      /* If both RUNPATH and RPATH are given, the latter is ignored.  */
      info[DT_RPATH] = NULL;
...
}
```

#### 3.2.3 Lazy Binding to Functions

Every dynamic executable or library inevitably accesses functions and data (two
primary types of symbols) from other dynamic libraries. However, the addresses
of these functions and data are not determined until the respective libraries
are loaded. Moreover, these addresses can vary with each program launch due to
Address Space Layout Randomization (ASLR) and differing runtime environments. In
ELF, these symbols are bound to their definitions by the dynamic linker and
stored in the Global Offset Table (GOT). The dynamic linker may support a
mechanism known as lazy binding, which delays resolving function references
until their first invocation. Notably, the dynamic linkers in musl and Android's
Bionic do not support lazy binding, unlike glibc, which does.

Consider the example of using the C standard function `printf` in the `main`
function:

```c
#include <stdio.h>

int main(void) {
  printf("Hello!\n");
  return 0;
}
```

Compiling this code into a dynamic executable with `clang -fPIE -o main main.c`
on x86-64 results in the delegation of the `printf` call to a specialized
trampoline within the Procedure Linkage Table (PLT), as demonstrated by the
assembly code:

```bash
Disassembly of section .plt:

0000000000001020 <.plt>:
    1020: ff 35 e2 2f 00 00             pushq   0x2fe2(%rip)            # 0x4008 <_GLOBAL_OFFSET_TABLE_+0x8>
    1026: ff 25 e4 2f 00 00             jmpq    *0x2fe4(%rip)           # 0x4010 <_GLOBAL_OFFSET_TABLE_+0x10>
    102c: 0f 1f 40 00                   nopl    (%rax)

0000000000001030 <printf@plt>:
    1030: ff 25 e2 2f 00 00             jmpq    *0x2fe2(%rip)           # 0x4018 <_GLOBAL_OFFSET_TABLE_+0x18>
    1036: 68 00 00 00 00                pushq   $0x0
    103b: e9 e0 ff ff ff                jmp     0x1020 <.plt>

Disassembly of section .text:

0000000000001140 <main>:
    1140: 55                            pushq   %rbp
    1141: 48 89 e5                      movq    %rsp, %rbp
    1144: 48 83 ec 10                   subq    $0x10, %rsp
    1148: c7 45 fc 00 00 00 00          movl    $0x0, -0x4(%rbp)
    114f: 48 8d 3d ae 0e 00 00          leaq    0xeae(%rip), %rdi       # 0x2004 <_IO_stdin_used+0x4>
    1156: b0 00                         movb    $0x0, %al
    1158: e8 d3 fe ff ff                callq   0x1030 <printf@plt>
    115d: 31 c0                         xorl    %eax, %eax
    115f: 48 83 c4 10                   addq    $0x10, %rsp
    1163: 5d                            popq    %rbp
    1164: c3                            retq
```

Each dynamic component generates a PLT function and a corresponding GOT entry
for every global function it invokes, including those defined within itself.
This arrangement supports interposition, allowing the definition of a global
function to be overridden by other implementations with the same name. In the
example provided, the runtime address of the function `printf` is retrieved from
the corresponding GOT entry within the PLT function, with the section `.got.plt`
(a dedicated GOT used by the PLT functions) starting at `0x4000` and measuring
32 bytes in size.

After the dynamic linker initializes a component:

- The first GOT entry is set to the runtime address of the `.dynamic` section,
although its specific purpose remains unclear.

- The second GOT entry stores a pointer (referred to as a handle) to an internal
data structure representing the current library, and this pointer is conveyed to
the dynamic linker to identify the library whose references require resolution.
This entry is employed only when lazy binding is active.

- The third GOT entry contains a pointer to a function (simplistically termed
the fixup routine, although it actually involves sequential invocations of
[\_dl\_runtime\_resolve][x86-64 dl runtime resolve] and
[\_dl\_fixup][dl fixup]), provided by the dynamic linker to resolve symbol
references, only when lazy binding is activated. For further details on setting
up the first three GOT entries on x86-64, refer to
[elf\_machine\_runtime\_setup][x86-64 elf machine runtime setup].

- Subsequent GOT entryies store the runtime address of the related global
functions. If lazy binding is enabled, the entry's value is initially set to the
the runtime address of the second half of its associated PLT function. This
function jumps into the first PLT function to perform preparatory symbol lookup
tasks and to call the fixup routine. Once the fixup routine completes its tasks,
the GOT entry is updated with the new value—the address of the target
function—enabling the function call. In contrast, without lazy binding, the
value is directly set to the target function's runtime adderss by the dynamic
linker. For details on how subsequent entries are configured, see
[elf\_machine\_lazy\_rel][x86-64 elf machine lazy rel]. According to
[elf\_machine\_lazy\_rel][x86-64 elf machine lazy rel] and
[struct link\_map\_machine][x86-64 struct link map machine], the offset from the
beginning of a PLT function to its second half measures `0x06`.

Concerning assocated PLT functions:

- The first PLT function (referred to as the preparatory PLT function) is used
to initiate a call to the fixup routine when lazy binding is enbled. Prior to
invoking the fixup routine, the value of the second GOT entry is retrieved and
stored on the stack. This value serves as a a handle to the current component.

- When lazy binding is enabled and any other PLT function is invoked for the
first time, it stores on the stack the index of the entry, associated with the
referenced function, in the PLT's reolocation table (e.g., the section
`.rela.plt`), then branches into the first PLT function. If the same PLT
function is invoked again, or if lazy binding is not enabled, it directly
branches to the referenced global function.

Furthermore, lazy binding can be disabled for a component through serveral
means:

- Setting the environment variable `LD_BIND_NOW`.

- Including a`DT_BIND_NOW` entry in the `.dynamic` section, setting the flag
`DF_BIND_NOW` in the `DT_FLAGS` entry, or setting the flag `DF_1_NOW` in the
`DT_FLAGS_1` entry.

- Loading the component by invoking `dlopen` with the `RTLD_NOW` flag.

In summary, lazy binding applies solely to function references and can be
implemented as follows:

1. All calls to global functions are initially routed to PLT functins, which
either conduct preparatory work for symbol lookp in cases of lazy binding or
proceed directly to the target function if the reference has already been
resolved.

2. The GOT entries are primarily utilized by the dynamic linker to store
addresses of global functions. However, with lazy binding, they are originally
set to the addresses of the codes performing preparatory work for symbol
resolution. After on-demand binding is completed, they are updated to point
directly to the related functions. The first few GOT entries are reserved for
use by the dynamic linker, and their number and functionality depend on the
hardware. For instance, on x86-64 and AArch64, the first GOT entry is used to
store the address of the `.dynamic` section, the second to maintain the handle
to the current component, and the third to position the address of the
preparatory PLT function. By contrast, for RISC-V, only two reserved entries
exist: the first one for the preparatory PLT function and the second one for the
handle.

## Future Work

1. Explore the sequence of events between the entry point of an executable and
the `main` function.

2. Determine the order in which loaded components are searched for the
definition of a symbol when resolving references to the symbol from another
component.

3. Investigate the operations performed by the dynamic linker concerning
thread-local storage.

4. Contrast the dynamic linking during process initialization with the approach
utilized by `dlopen`.

5. Examine the memory management strategies employed by glibc's rtld prior to
libc being loaded, noting that functions like `malloc`, `calloc`, and `free` from libc
are unavailable.

6. Explore alternative implementations of PLT functions. According to glibc's
[elf\_machine\_lazy\_rel][x86-64 elf machine lazy rel], at least two
alternatives exist; one such alternative, known as prelink, has been discuessed
in this article.

## Appendix

### Linux execv

The diagram below illustrates the call graph for the `execv` function:

```
execv
└── do_execve
    └── do_execveat_common
        ├── alloc_bprm
        │   └── bprm_mm_init: create a new mm_struct and populate it with a temporary stack
        │       ├── mm_alloc: allocate and initialize an mm_struct
        │       │   └── mm_init
        │       └── __bprm_mm_init
        ├── bprm_stack_limits: set stack limit
        ├── copy_string_kernel: used to copy filename to the process stack
        ├── copy_strings: used to copy argv/envp to the process stack
        └── bprm_execve
            └── exec_binprm
                └── search_binary_handler
                    ├── prepare_binprm: read the first BINPRM_BUF_SIZE bytes from the file
                    └── load_elf_binary
                        ├── load_elf_phdrs
                        │   └── elf_read
                        ├── elf_read
                        ├── parse_elf_properties: parse ELF section '.note.gnu.property'
                        │   └── parse_elf_property: parse description data in '.note.gnu.property'
                        │       └── arch_parse_elf_property
                        ├── arch_check_elf
                        ├── begin_new_exec: do process-related clean/initialization
                        │   ├── set_mm_exe_file: change a reference to the mm's executable file
                        │   └── exec_mmap: release all of the old mmap stuff
                        ├── setup_new_exec
                        ├── setup_arg_pages: finalize the stack vm_area_struct
                        ├── make_prot: determine permissions of pages for an ELF loadable segment
                        ├── elf_load
                        │   └── elf_map
                        ├── load_elf_interp
                        ├── create_elf_tables: initialize argc, argv, env and auxv into the stack
                        ├── ELF_PLAT_INIT: a macro for initialization on x86-64 hardwares
                        │   └── elf_common_init
                        └── finalize_exec: initialize (general purpose) registers on x86-64 hardwares
```

Related source code links are provided for in-depth examination:

- [bprm\_mm\_init](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L380-L408)

- [mm\_alloc](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/kernel/fork.c#L1326-L1336)

- [mm\_init](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/kernel/fork.c#L1254-L1321)

- [\_\_bprm\_mm\_init](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L255-L307)

- [bprm\_stack\_limits](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L489-L528)

- [copy\_string\_kernel](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L630-L664)

- [copy\_strings](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L535-L625)

- [prepare\_binprm](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1727-L1733)

- [parse\_elf\_properties](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L762-L817)

- [parse\_elf\_property](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L714-L756)

- [begin\_new\_exec](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1271-L1451)

- [set\_mm\_exe\_file](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/kernel/fork.c#L1404-L1430)

- [exec\_mmap](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1007-L1068)

- [setup\_new\_exec](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1476-L1492)

- [setup\_arg\_pages](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L761-L880)

- [make\_prot](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L611-L624)

- [finalize\_exec](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/exec.c#L1496-L1502)

- [ELF\_PLAT\_INIT](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/arch/x86/include/asm/elf.h#L169)

- [elf\_common\_init](https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/arch/x86/include/asm/elf.h#L156-L167)

### A2. Entry Point of glibc's rtld 

The call graph for `_start` of glibc's rtld is depicted below:

```
_start: the entry point of rtld which is the first function executed in user space
└── _dl_start: initialize rtld and the executable, and load dependencies
    ├── elf_machine_load_address: figure out the run-time load address of the dynamic linker itself
    ├── elf_machine_dynamic: get the link-time address of _DYNAMIC
    ├── elf_get_dynamic_info: read the dynamic linker's dynamic section and fill in the info array
    │   └── dl_relocate_ld: check wheter entries .dynamic should be relocated (true on x86-64)
    ├── ELF_DYNAMIC_RELOCATE: do relocation for rtld itself so that its own function calls and data can be accessed via GOT
    │   ├── elf_machine_runtime_setup: set up lazy binding by populating the first few GOT entries used by PLT
    │   ├── ELF_DYNAMIC_DO_RELR: do RELR (a compact format compared with REL and RELA) relocation if existing
    │   ├── ELF_DYNAMIC_DO_REL: do REL relocation
    │   │   └── _ELF_DYNAMIC_DO_RELOC: a template macro expand to do REL relocation
    │   │       └── elf_dynamic_do_Rel: do REL relocation for GOT and PLTGOT (if not lazy)
    │   └── ELF_DYNAMIC_DO_RELA: do RELA relocation
    │       └── _ELF_DYNAMIC_DO_RELOC: a template macro expand to do RELA relocation
    │           └── elf_dynamic_do_Rela: do RELA relocation for GOT and PLTGOT (if not lazy)
    └── _dl_start_final: the second half of _dl_start
        ├── _dl_setup_hash: set up the dynamic linker's symbol hash table
        └── _dl_sysdep_start: do OS-dependent initialization
            ├── _dl_sysdep_parse_arguments: keep records of command-line arguments, environment variabels and the aux vectory
            │   └── _dl_parse_auxv: keep records of the aux vector
            ├── DL_PLATFORM_INIT: a macro to invoke the function that performs platform-dependent initialization for rtld
            │   └── dl_platform_init: perform rtld's initialization that is specific to x86-64
            │       └── _dl_x86_init_cpu_features: detect and record x86-64 CPU features
            │           └── init_cpu_features: detect CPU features
            └── dl_main: initialize the executable, and load dependency libraries
                ├── process_envvars: process any "LD_" environment variables
                ├── _dl_new_object: allocate link_map for the main executable
                ├── _dl_add_to_namespace_list: add the main executable to the object list identified by LM_ID_BASE
                ├── rtld_setup_main_map
                │   ├── _dl_process_pt_gnu_property: process PT_GNU_PROPERTY of the executable
                │   └── _dl_process_pt_note: process PT_NOTE of the executable
                ├── elf_get_dynamic_info: called to fill the main executable's dynamic info array
                ├── _dl_setup_hash: read the main executable's symbol hash table
                ├── setup_vdso: set up vDSO
                │   ├── _dl_new_object: allocate link_map for vDSO
                │   ├── elf_get_dynamic_info: called to fill vDSO's dynamic info array
                │   ├── _dl_setup_hash: read vDSO's symbol hash table
                │   └── _dl_add_to_namespace_list: add vDSO to the object list identified by LM_ID_BASE
                ├── setup_vdso_pointers: set up the vDSO function pointers
                │   └── dl_vdso_vsym: resolve symbols in vDSO
                │       └── _dl_lookup_symbol_x: resolve a symbol with version information in vDSO
                │           ├── _dl_new_hash: calculate the hash for a symbol name in GNU style which is the used in '.gnu.hash'
                │           └── do_lookup_x: try to resolve a versioned symbol in vDSO
                │               ├── check_match: check if two symbol match by comparing a symbol's name and version
                │               │   └── _dl_name_match_p: check if a version's file name matches a link_map
                │               ├── _dl_elf_hash: calculate ELF-style hash of a symbol
                │               ├── dl_symbol_visibility_binds_local_pl: check if the visibility of a symbol is local or hidden, which means it is a local symbol
                │               └── do_lookup_unique: look up a STB_GNU_UNIQUE symbol in the current namespace
                ├── call_init_paths: initialize the data structures for the search paths for shared objects
                ├── _dl_debug_initialize: initialize _dl_debug_extended (used by debugger) for namespace LM_ID_BASE
                ├── elf_setup_debug_entry: set up DT_DEBUG of '.dynamic' of the executable so that a debugger can access the structure 'r_debug'
                ├── handle_preload_list: load shared libraries specified by either the command-line arg '--preload' and the environment variable 'LD_PRELOAD' to the executable's global scope
                │   └── do_preload: load a pre-loaded shared library
                ├── do_preload: load shared libraries specified by '/etc/ld.so.preload'
                ├── _dl_map_object_deps: load all libraries specified by DT_NEEDED entries and all transitive dependencies of the executable
                │   ├── preload: load the link_map dependencies of which will be loaded subsequently
                │   ├── _dl_map_object: map the object specified by DT_NEEDED
                │   │   └── _dl_map_object_from_fd: map the object into memory with a valid fd
                │   │       ├── _dl_new_object: allocate link_map for for the needed shared object
                │   │       ├── _dl_map_segments: map loadable segments into memory
                │   │       │   └── _dl_map_segment: map loadable segments as a whole region of memory
                │   │       ├── elf_get_dynamic_info: fill the new mapped object's dynamic info array
                │   │       ├── _dl_process_pt_gnu_property: process PT_GNU_PROPERTY of the new mapped object
                │   │       ├── _dl_process_pt_note: process PT_NOTE of the new mapped object
                │   │       ├── _dl_setup_hash: set up the symbol hash table of the new mapped object
                │   │       ├── _dl_add_to_namespace_list: add the new object to the loader's namespace
                │   │       ├── _dl_debug_update: get the 'r_debug' of the current namespace, and update 'r_debug.r_map' if needed
                │   │       └── _dl_debug_state: notify the debugger a shared object is going to be added
                │   └── _dl_sort_maps: sort link maps according to dependencies
                ├── version_check_doit: check whether all libraryies needed by the executable are in the versions needed
                │   └── _dl_check_all_versions: check needed versions of libraries in the default namespace owned by the executable
                │       └── _dl_check_map_versions: check needed versions of a library, and initialize the version table
                │           └── find_needed: find the required object in the current namespace or the search list according to the name
                │           └── match_symbol: check whether the version symbol matches
                ├── _dl_relocate_object: do relocation for libc
                │   ├── ELF_DYNAMIC_RELOCATE: do relocation for GOT and other data
                │   └── _dl_protect_relro: set the memory region specified by PT_GNU_RELRO to read-only
                ├── _dl_relocate_object: do relocation for the executable and its dependencies
                ├── __rtld_malloc_init_real: if rtld is used by explicitly as a shared library, resolve rtld's memory-related functions (calloc', 'free', 'malloc' and 'realloc') in the executable's scopes
                ├── __rtld_mutex_init: if rtld is used by explicitly as a shared library, resolve rtld's pthread-lock-related functions in the executable's scopes
                ├── _dl_relocate_object: if rtld is used by explicitly as a shared library, resolve rtld's required symbols in the executable's scopes
                ├── _dl_call_libc_early_init: perform early libc initialization
                ├── _dl_debug_update: initialize 'r_debug' for the namespace of the executable if not initialized
                └── _dl_debug_state: notify the debugger that all dependencies of the executable are loaded
```

The call graph of `_dl_start_user`:

```
_dl_start_user: do the last several works, and jump to the entry point of the executable
├── RTLD_START_ENABLE_X86_FEATURES: enable shadow stack
└── _dl_init: run initializes for the executable and its dependencies
    └── call_init: call initializers for a shared object
        └── DL_CALL_DT_INIT: call the function referenced by DT_INIT if present
```

Related source code Links:

- [\_start](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L141-L143)

- [\_dl\_start](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/rtld.c#L515-L585)

- [struct rtld\_global](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/ldsodefs.h#L303-L502)

- [struct rtld\_global.\_dl\_rtld\_map](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/ldsodefs.h#L396)

- [elf\_machine\_load\_address][x86-64 elf machine load address]

- [elf\_machine\_dynamic][elf machine dynamic]

- [elf\_get\_dynamic\_info][elf get dynamic info]

- [struct link\_map][struct link map]

- [struct Elf64\_Dyn](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/elf.h#L869-L877)

- [dl\_relocate\_ld](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/ldsodefs.h#L75-L80)

- [ELF\_DYNAMIC\_RELOCATE](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dynamic-link.h#L191-L200)

- [\_dl\_start\_final](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/rtld.c#L446-L507)

- [\_dl\_setup\_hash](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-setup_hash.c#L23-L63)

- [\_dl\_sysdep\_start](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/unix/sysv/linux/dl-sysdep.c#L98-L143)

- [\_dl\_sysdep\_parse\_arguments](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/unix/sysv/linux/dl-sysdep.c#L75-L96):

- [\_dl\_parse\_auxv](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/unix/sysv/linux/dl-parse_auxv.h#L29-L63):

- [DL\_PLATFORM\_INIT](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L201)

- [dl\_platform\_init](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L203-L215)

- [\_dl\_x86\_init\_cpu\_features](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86/dl-get-cpu-features.c#L35-L74)

- [init\_cpu\_features](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86/cpu-features.c#L750-L1250)

- [dl\_main](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/rtld.c#L1341-L2407)

- [process\_envvars](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/rtld.c#L2758-L2768)

- [\_dl\_new\_object](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-object.c#L56-L263):

- [\_dl\_add\_to\_namespace\_list](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-object.c#L29-L51):

- [rtld\_setup\_main\_map][rtld setup main map]

- [\_dl\_process\_pt\_gnu\_property](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-load.c#L865-L929)

- [\_dl\_process\_gnu\_property](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86/dl-prop.h#L239-L261)

- [\_dl\_process\_pt\_note](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86/dl-prop.h#L232-L237)

- [\_dl\_process\_property\_note](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86/dl-prop.h#L138-L230)

- [setup\_vdso][setup vdso]

- [setup\_vdso\_pointers](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/unix/sysv/linux/dl-vdso-setup.h#L23-L53)

- [dl\_vdso\_vsym](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/unix/sysv/linux/dl-vdso.h#L37-L56)

- [\_dl\_lookup\_symbol\_x](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-lookup.c#L766-L892)

- [\_dl\_new\_hash](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/dl-new-hash.h#L65-L107)

- [do\_lookup\_x](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-lookup.c#L340-L522)

- [check\_match](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-lookup.c#L58-L162)

- [\_dl\_name\_match\_p](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-misc.c#L66-L83)

- [\_dl\_elf\_hash](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/dl-hash.h#L26-L73)

- [dl\_symbol\_visibility\_binds\_local\_p](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/ldsodefs.h#L136-L141)

- [do\_lookup\_unique](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-lookup.c#L210-L335)

- [call\_init\_paths](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-main.h#L106-L111)

- [\_dl\_debug\_initialize](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-debug.c#L54-L107)

- [struct r\_debug](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/link.h#L40C1-L63)

- [struct r\_debug\_extended](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/link.h#L74C8-L84)

- [version\_check\_doit](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/rtld.c#L672-L680)

- [\_\_rtld\_malloc\_init\_real](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-minimal.c#L74-L103)

- [\_r\_debug](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-debug-symbols.S#L38)

- [\_r\_debug\_extended](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-debug-symbols.S#L39C1-L39C18)

- [elf\_setup\_debug\_entry](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/dl-debug.h#L25-L31)

- [handle\_preload\_list](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/rtld.c#L866-L895)

- [do\_preload](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/rtld.c#L802-L831)

- [\_dl\_map\_object\_deps](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-deps.c#L139-L574)

- [preload](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-deps.c#L125-L137)

- [\_dl\_map\_object](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-load.c#L1944-L2244)

- [\_dl\_map\_object\_from\_fd](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-load.c#L935-L1518)

- [\_dl\_map\_segments](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-map-segments.h#L74-L202)

- [\_dl\_map\_segment](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-map-segments.h#L24-L65)

- [\_dl\_debug\_update](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-debug.c#L36-L48)

- [\_dl\_debug\_state](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-debug.c#L114-L117)

- [\_dl\_sort\_maps](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-sort-maps.c#L295-L310)

- [\_dl\_check\_all\_versions](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-version.c#L390-L401)

- [\_dl\_check\_map\_versions](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-version.c#L153-L387)

- [find\_needed](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-version.c#L29-L49)

- [match\_symbol](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-version.c#L52-L150)

- [\_dl\_relocate\_object](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-reloc.c#L204-L350)

- [ELF\_DYNAMIC\_RELOCATE](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dynamic-link.h#L191-L200)

- [elf\_machine\_runtime\_setup][x86-64 elf machine runtime setup]

- [ELF\_DYNAMIC\_DO\_RELR](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dynamic-link.h#L153-L178)

- [ELF\_DYNAMIC\_DO\_REL](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dynamic-link.h#L138-L139)

- [\_ELF\_DYNAMIC\_DO\_RELOC](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dynamic-link.h#L81-L128)

- [elf\_dynamic\_do\_Rel](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/do-rel.h#L42-L221)

- [\_dl\_protect\_relro](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-reloc.c#L353-L370)

- [\_\_rtld\_mutex\_init](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/nptl/dl-mutex.c#L28-L53)

- [\_dl\_call\_libc\_early\_init](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-call-libc-early-init.c#L25-L41)

- [\_dl\_start\_user](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L144-L174)

- [RTLD\_START\_ENABLE\_X86\_FEATURES](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/unix/sysv/linux/x86_64/dl-cet.h#L80-L100)

- [\_dl\_init](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-init.c#L79-L127)

- [call\_init](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-init.c#L25-L76)

- [DL\_CALL\_DT\_INIT](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/ldsodefs.h#L117-L118)

### A3. Debugging glibc's rtld

Fetch the source code:

```
$ git clone https://sourceware.org/git/glibc.git
$ cd glibc
$ git checkout master
```

Create directories for building and installation:

```
$ mkdir build install
```

Build and install:

```
$ cd build
$ ../glibc/configure --enable-shared --prefix ../install
$ make -j $(nproc)
$ make install
```

Create a dynamic executable, and use just built rtld as the loader with
`GLIBC_INSTALL_DIR` being the directory where output files of building are
installed:

```
$ clang -o main -Wl,-dynamic-linker,$GLIBC_INSTALL_DIR/lib/ld-linux-x86-64.so.2 main.c
```

Debug the executable with GDB:

```
$ gdb --tui ./main
```

Within the GDB shell:

```
(gdb) b _start
```

## References

[1] Wenbo Shen's [Understanding Linux Execve System Call][wenbo shen execve]

[wenbo shen execve]: https://wenboshen.org/posts/2016-09-15-kernel-execve

[2] [exec(3p) — Linux manual page][linux man exec]

[linux man exec]: https://man7.org/linux/man-pages/man3/exec.3p.html

[3] [Implementing input/output redirection in a Linux shell using C][stackoverflow
io redirection]

[stackoverflow io redirection]: https://stackoverflow.com/questions/35569673/implementing-input-output-redirection-in-a-linux-shell-using-c

[4] [Memory layout of argc, argv, envp][stackoverflow elf argc argv envp]

[stackoverflow elf argc argv envp]: https://stackoverflow.com/questions/23599762/memory-layout-of-argc-argv-envp

[5] [ELF x86-64-ABI psABI][elf amd64 psabi]

[elf amd64 psabi]: https://gitlab.com/x86-psABIs/x86-64-ABI

[6] [zSeries ELF Application Binary Interface Supplement, Process initialization][zeries elf process initialization]

[zeries elf process initialization]: https://refspecs.linuxfoundation.org/ELF/zSeries/lzsabi0_zSeries/x895.html

[7] [ELF: symbol lookup via DT_HASH][elf hash]

[elf hash]: https://flapenguin.me/elf-dt-hash

[8] [ELF: better symbol lookup via DT_GNU_HASH][elf gnu hash]

[elf gnu hash]: https://flapenguin.me/elf-dt-gnu-hash

[9] MaskRay's [.init, .ctors, and .init\_array][init array]

[init array]: https://maskray.me/blog/2021-11-07-init-ctors-init-array

[x86-64 elf machine runtime setup]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L69-L131

[elf machine dynamic]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L59-L64

[elf get dynamic info]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/get-dynamic-info.h#L28-L188

[create elf tables]: https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c#L155-L349

[linux binfmt elf]: https://github.com/torvalds/linux/blob/9b62e02e63363f5678d5598ee7372064301587f7/fs/binfmt_elf.c

[rtld setup main map]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/rtld.c#L1110-L1283

[struct link map]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/include/link.h#L95-L348

[setup vdso]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/setup-vdso.h#L19-L109

[x86-64 elf machine load address]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L51-L56

[x86-64 elf machine lazy rel]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-machine.h#L497-L544

[x86-64 struct link map machine]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86/linkmap.h

[x86-64 dl runtime resolve]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/x86_64/dl-trampoline.h#L37

[dl fixup]: https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-runtime.c#L39-L163
