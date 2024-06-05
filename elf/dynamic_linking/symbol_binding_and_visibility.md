# Symbol Resolution: Binding and Visibility

## Introduction

Programmers utilize symbols to define and reference foundamental programming
elements like varaibles and functions. Nowadays, linkers are tasked with
logically and appropriately connecting these symbol definitions and references
. As per *Linkers and Loaders* [4], linking involves the binding of symbol
references to addresses. This process occurs during the creation of executables
and shared libraries, as well as when they are loaded either for process
initialization or on-demand at runtime. To enhance flexibility, programmer can
use symbol attributes to tailor the linking process to their specific needs. In
this article, I will discuss how the binding and visibility attributes
of an ELF symbol influence both static and dynamic linking. The article is
structured to provide overarching guides for quick reference followed by
detailed case studies.

The following assumptions are made about the readers:

- Readers are presumed to be familiar with the general steps underlying the
compilation of C/C++ source files into an executable or a shared object in ELF
formats, encompassing the processes executed by a compiler driver (e.g. GCC and
clang), often without the user's explicit awareness. This process essentially
entails compiling each file separately (known as a compilation unit or a
translation unit) into a reolocatable file, and then linking them together with
pre-created libraries into an executable file. Often, the term "compilation" is
also referred to this entire process.

- From a programmer's perspective, ELF files contain various sections for
different purposes, such as sections for machine code and for initialized global
data. It is assumed that readers have experience using command-line tools (e.g.
`llvm-objdump`, `llvm-readel` and `hexdump`) to explore the content of an ELF
file.

- Readers are expected to understand the general sequence of events that occur
before the user's code (e.g. the C entry point `main`) is executed at runtime.
For dynamic executables, they are partially loaded by the OS kernel and further
initialized by a dynamic linker to resolve nay shared-library dependencies.

## General Rules

In an ELF file, there are usually one or more symbol tables where information
(e.g. names and types) of symbols (either referenced or defined) is stored. In
such a symbol table, the structure of a symbol can be represented as follows:

```c
typedef struct
{
  Elf64_Word    st_name;              /* Symbol name (string tbl index) */
  unsigned char st_info;              /* Symbol type and binding */
  unsigned char st_other;             /* Symbol visibility */
  Elf64_Section st_shndx;             /* Section index */
  Elf64_Addr    st_value;             /* Symbol value */
  Elf64_Xword   st_size;              /* Symbol size */
} Elf64_Sym;
```

The two attributes we are concerned with are encoded in the fields, `st_info`
and `st_other`. Please refer to the section *Symbol Table* in [1] for details on
how these attributes are recorded. There are three main binding attributes (others are
reserved for either OS-specific or processor-specific semantics):

- `STB_LOCAL`: a symbol with this attribute is referred to as a local symbol.

- `STB_GLOBAL`: a global and strong symbol.

- `STB_WEAK`: a global but weak symbol.

There are also four different visibility attributes:

- `STV_DEFAULT`: global symbol definitions are visible to other objects while
local ones are not.

- `STV_HIDDEN`: not visible outside the defining shared library.

- `STV_INTERNAL`: similar to `STV_HIDDEN` but with additional
processor-supplemented constraints.

- `STV_PROTECTED`: such symbols are visible outside; however, references from a
defining shared library must be resolved within the same file.

Typically, address binding is handled by either a static linker at compile-time
or a dynamic linker at runtime. At link-time, when multiple relocatable files
are linked together into an executable or a shared library, there are two
typical cases where the binding attribute of a symbol comes into play.

The first scenario involves multiple definitions with the same symbol name. The
consequence of linking differs by the binding attributes and the type of files
where the definitions are located, summarized in the following table:

| Relocatable Files | Archive Libraries | Shared Libraries | Definition Bound to | Case Study |
| ----------------- | ----------------- | ---------------- | ------------------- | ---------- |
| multiple strong definitions | - | - | a link-time error due to conflicts | Case 1 |
| one strong definition and multiple weak ones | - | - | the strong definition | Case 3 |
| multiple local definitions | - | - | the definition in its own relocatable file | Case 2 |
| one strong/weak definition | zero or more | zero or more | the one in reloctable files | Case 4 |

The second scenario addresses unresolved references. What happens to an
unresolved symbol reference varies by its binding attribute and the type of the
enclosing file at the time of linking, with associated consequences at runtimes
as well. See case 5 for details.

| Binding | Target File | Error from Static Linker | Error from Dynamic Linker |
| ------- | ----------- | ------------------------ | ------------------------- |
| `STB_STRONG` | executable | Y | Y |
| `STB_STRONG` | shared object | N | Y |
| `STB_WEAK` | excutable or shared object| N | N |

Next, we will discuss how static and dynamic linking are influenced by the
visibility attributes. In fact, these attributes do not have impact on how
symbol references are resolved by a static linker. However, they do affect
whether the related symbol is exported in the created executables or shared
libraries. Furthermore, runtime symbol lookup is also influenced by these
attributes. The following table summarizes behaviors static and dynamic linkers
regarding the visbility attributes.

| Binding Attribute | Visibility Attribute | Effectively Visible Outside | Can be Interposed or Overridden at Runtime | Note |
| ----------------- | -------------------- | --------------------------- | ------------------------------------------ | ---- |
| `STB_STRONG` | `STV_DEFAULT` | Y | Y | |
| `STB_WEAK` | `STV_DEFAULT` | Y | Y | |
| `STB_LOCAL` | `STV_DEFAULT` | N | N | only the combination of `STB_LOCAL` and `STV_DEFAULT` is meaningful |
| `STB_STRONG` | `STV_PROTECTED` | Y | N | references are resolved in the containing shared object or executable at runtime |
| `STB_STRONG` | `STV_HIDDEN` | N | N | should be reduced to `STB_LOCAL` in shared objects |
| `STB_STRONG` | `STV_INTERNAL` | N | N | should be reduced to `STB_LOCAL` in shared objects |

Please refer to case 6 for runtime interposition. Two things are worth noting:

- This section does not aim (possibly could not) to be an exhaustive list on how
linkers handle the binding and visibitlity attributes. The above rules are
guided by ELF specifications like [1] and great guidelines [2, 3] from
developers for Solaris' static and dynamic linkers. It would be helpful to read
these documents for a broader understanding of symbol resolution not covered in
this article.

- The rules in this section cover what static and dynamic linkers do by default.
In my opinion, these default behaviors are probably the best choice for common
use cases. However, in some rare schenarios, they may not be desirable.
Fortunately, most of static and dynamic linkers offer some options (e.g.,
command-line arguments and environment variables) to tweak their handling.
Although some options are loosely convered in this article, they remain not the
focus. Please refer to the tools you are using for corresponding manuals.

## Case Studies

### Case 1: Symbol Conflicts

The source code is located in the directory
[src/symbol/conflict](/elf/dynamic_linking/src/symbol/conflict).

Given multiple relocatable files to create an executable or a shared object,
there may be multiple strong definitions for a symbol across these files. For
example, suppose there is a strong definition for the function `greet` in
`api.o`, and another in `main.o`. They will fail to link due to symbol
conflicts.

```c
// api.c
#include <stdio.h>

void greet(void) {
  printf("Hello, this is %s from %s\n", __func__, __FILE__);
}

// main.c
#include <stdio.h>

void greet(void) {
  printf("Hi, this is %s from %s\n", __func__, __FILE__);
}

int main(void) {
  greet();
  return 0;
}
```

If you try to compile and link them, your linker should report errors similar to
the ones below:

```bash
$ make main
cc    -c -o main.o main.c
cc    -c -o api.o api.c
cc   main.o api.o   -o main
/usr/bin/ld: api.o: in function `greet':
api.c:(.text+0x0): multiple definition of `greet'; main.o:main.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
make: *** [<builtin>: main] Error 1
$ make libmain.so
cc -shared -fPIC -o libmain.so main.o api.o
/usr/bin/ld: api.o: in function `greet':
api.c:(.text+0x0): multiple definition of `greet'; main.o:main.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
make: *** [Makefile:6: libmain.so] Error 1
```

If relocatable files are linked against archive or shared libraries, there is
one (or more) strong symbol definition in one of the relocatable files with
multiple alternative definitions for the same symbol in libraries, there will be
no conflicts. This is due to a feature called interposition, which allows a
definition to override others in libraries during linking at compile-time as
well as runtime. See Case 4 for more about runtime interposition). The
following output demonstrates link-time interposition:

```bash
$ make greet
cc -shared -fPIC -o libapi.so api.o
cc -o greet -L. -lapi main.o
```

### Case 2: Local Definitions

The source code is located in the directory
[src/symbol/local](/elf/dynamic_linking/src/symbol/local).

Suppose a function defined as `static void print(void)` exists both in `api.c`
and `main.c`. As shown below, two `STB_LOCAL` definitions for `print` are
retained in the executable created from `api.c` and `main.c`, with references in
each file bound to the definition within the same file.

The symbol table of `main` is as follows:

```bash
Symbol table '.symtab' contains 42 entries:
   Num:    Value          Size Type    Bind   Vis       Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT   UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT   ABS Scrt1.o
     2: 000000000000038c    32 OBJECT  LOCAL  DEFAULT     4 __abi_tag
     3: 0000000000000000     0 FILE    LOCAL  DEFAULT   ABS crtstuff.c
     4: 0000000000001090     0 FUNC    LOCAL  DEFAULT    16 deregister_tm_clones
     5: 00000000000010c0     0 FUNC    LOCAL  DEFAULT    16 register_tm_clones
     6: 0000000000001100     0 FUNC    LOCAL  DEFAULT    16 __do_global_dtors_aux
     7: 0000000000004010     1 OBJECT  LOCAL  DEFAULT    26 completed.0
     8: 0000000000003dc0     0 OBJECT  LOCAL  DEFAULT    22 __do_global_dtors_aux_fini_array_entry
     9: 0000000000001140     0 FUNC    LOCAL  DEFAULT    16 frame_dummy
    10: 0000000000003db8     0 OBJECT  LOCAL  DEFAULT    21 __frame_dummy_init_array_entry
    11: 0000000000000000     0 FILE    LOCAL  DEFAULT   ABS main.c
    12: 0000000000001149    51 FUNC    LOCAL  DEFAULT    16 print
    13: 000000000000201e     6 OBJECT  LOCAL  DEFAULT    18 __func__.0
    14: 0000000000000000     0 FILE    LOCAL  DEFAULT   ABS api.c
    15: 000000000000119a    51 FUNC    LOCAL  DEFAULT    16 print
```

Running `main` should produce output like the following:

```bash
$ ./main
function print in main.c
function print in api.c
```

Moreover, if two local functions are equivalent, one of them can be eliminated
to reduce code size and potentially improve runtime performance.

### Case 3: Weak Definitions

The source code is situated in the directory
[src/symbol/weak](/elf/dynamic_linking/src/symbol/weak).

Assume there are multiple weak definitions for a symbol across various
relocatable files. According to the System V Application Binary Interface [1],
weak symbols are similar to strong symbols but are treated with lower precedence
in the following scenarios:

- When both a strong symbol definition and a weak symbol with the same name
exist, this does not result in an error. The linker prioritizes the strong
symbol. Similarly, if both a common symbol (where the value of the field
`st_shndx` is `SHN_COMMON`) and a weak symbol defition share the same name, the
linker should prefer the common symbol definition.

Consider the following example where the function `greet` is defined in three
locations each with different binding attributes.

| File Name | Binding Attribute |
| --------- | ----------------- |
| `impl_strong.c` | `STB_GLOBAL` |
| `impl_weak1.c` | `STB_WEAK` |
| `impl_weak2.c` | `STB_WEAK` |

And in `main.c`, the function is referenced. It is permissible to compile these
four files to create an executable, where the strong definition is utilzed, as
demonstrated below:

```c
$ ./main_strong
Hi, I'm strong greet from impl_strong.c
```

Should only one weak definition remains, the linker has the discretion to select
any of them. With GNU ld, the sequence in which files are input affects the
outcome:

```bash
$ cc -o main_weak1 main.o impl_weak1.o impl_weak2.o
$ cc -o main_weak2 main.o impl_weak2.o impl_weak1.o
$ ./main_weak1
Hi, I'm weak greet from impl_weak1.c
$ ./main_weak2
Hi, I'm weak greet from impl_weak2.c
```

Unlike `STB_LOCAL`, there is a singular definition of `greet` in the executable.

### Case 4: Archive Libraries

The source code is housed in the directory
[src/symbol/archive](/elf/dynamic_linking/src/symbol/archive).

An archive library is effectively a collection of relocatable files and can be
created by `ar` along with a specified list of relocatable files. In this
section, I explore archive libraries as a separate case study due to their
unique characteristics:

- An unresolved reference to a weak symbol in a relocatable file does not cause 
an object containing the definition to be extracted from an archive library.
Such references may remain unreolsved silently, as memtioned in Case 3. This
behavior accounts for one of the two primary distinctions between strong and
weak symbols outlined in the ELF gABI [1]. By contrast, an object within an
archive will be extracted if a symbol definition within it satisfies a
reference.

To illustrate this more clearly, consider the following code:

```c
// greet.c -> greet.o -> libapi.a
#include <stdio.h>

void greet(void) {
  printf("Hello! This is %s of %s\n", __func__, __FILE__);
}

// main.c -> main
// linked with libapi.a
__attribute__((weak)) extern void foo(void);

int main(void) {
  greet();
  foo();
  return 0;
}
```

The implementation of `greet` is provided by `libapi.a`, while the use of `foo`
remains undefined, leading to a segmentation fault as demonstrated by the output
below:

```bash
$ ./main
Hello! This is greet of greet.c
Segmentation fault (core dumped)
```

### Case 5: Undefined Symbols

The source code is stored in the directory
[src/symbol/undefined](/elf/dynamic_linking/src/symbol/undefined).

It was quite surprising to learn that the specification [1] allows unresolved
weak symbols, mandating that they assume a zero value.

> When the link editor searches archive libraries [see \`\`Archive File'' in
> Chapter 7], it extracts archive members that contain definitions of undefined
> global symbols. The member's definition may be either a global or a weak
> symbol. The link editor does not extract archive members to resolve undefined
> weak symbols. Unresolved weak symbols have a zero value.]

> First, all of the non-default visibility attributes, when applied to a symbol
> reference, imply that a definition to satisfy that reference must be provided
> within the current executable or shared object. If such a symbol reference has
> no definition within the component being linked, then the reference must have
> STB\_WEAK binding and is resolved to zero.

This appears to imply that an unresolved weak variable is allocated and zero
initialized, and a reference to a weak function is effectively a null pointer.
While this approach seems acceptable for variables at runtime, it can lead to
issues for functions. Let's examine how GCC and GNU ld handle these references.

```c
#include <stdio.h>

__attribute__((weak)) extern int a;

__attribute__((weak)) extern void non_existing(void);

int main(void) {
  printf("a = %d\n", a);
  non_existing();
  return 0;
}
```

Altough the above code compiles, running it results in a segmentation fault. The
assembly code reveals that the address is initially fetched from the
corresponding entry in the Global Offset Tbale (GOT), and then memory at that
address is accessed. The machine code for the function `main` is as follows:

```bash
0000000000001169 <main>:
    1169:       f3 0f 1e fa             endbr64 
    116d:       55                      push   %rbp
    116e:       48 89 e5                mov    %rsp,%rbp
    1171:       48 8b 05 70 2e 00 00    mov    0x2e70(%rip),%rax        # 3fe8 <a@Base>
    1178:       8b 00                   mov    (%rax),%eax
    117a:       89 c6                   mov    %eax,%esi
    117c:       48 8d 05 81 0e 00 00    lea    0xe81(%rip),%rax        # 2004 <_IO_stdin_used+0x4>
    1183:       48 89 c7                mov    %rax,%rdi
    1186:       b8 00 00 00 00          mov    $0x0,%eax
    118b:       e8 d0 fe ff ff          call   1060 <printf@plt>
    1190:       e8 db fe ff ff          call   1070 <non_existing@plt>
    1195:       b8 00 00 00 00          mov    $0x0,%eax
    119a:       5d                      pop    %rbp
    119b:       c3                      ret
```

The dynamic symbol table of the executable lists two references to `a` and
`non_existing` respectively:

```bash
Symbol table '.dynsym' contains 9 entries:
   Num:    Value          Size Type    Bind   Vis       Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT   UND 
     1: 0000000000000000     0 FUNC    GLOBAL DEFAULT   UND __libc_start_main@GLIBC_2.34
     2: 0000000000000000     0 NOTYPE  WEAK   DEFAULT   UND _ITM_deregisterTMCloneTable
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT   UND printf@GLIBC_2.2.5
     4: 0000000000000000     0 NOTYPE  WEAK   DEFAULT   UND __gmon_start__
     5: 0000000000000000     0 NOTYPE  WEAK   DEFAULT   UND non_existing
     6: 0000000000000000     0 NOTYPE  WEAK   DEFAULT   UND a
     7: 0000000000000000     0 NOTYPE  WEAK   DEFAULT   UND _ITM_registerTMCloneTable
     8: 0000000000000000     0 FUNC    WEAK   DEFAULT   UND __cxa_finalize@GLIBC_2.2.5
```

The relocation sections, `.rela.dyn` and `.rela.plt`, store the necessary
relocation information for these symbols:

```bash
Relocation section '.rela.dyn' at offset 0x590 contains 9 entries:
    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
0000000000003da8  0000000000000008 R_X86_64_RELATIVE                 1160
0000000000003db0  0000000000000008 R_X86_64_RELATIVE                 1120
0000000000004008  0000000000000008 R_X86_64_RELATIVE                 4008
0000000000003fd0  0000000100000006 R_X86_64_GLOB_DAT      0000000000000000 __libc_start_main@GLIBC_2.34 + 0
0000000000003fd8  0000000200000006 R_X86_64_GLOB_DAT      0000000000000000 _ITM_deregisterTMCloneTable + 0
0000000000003fe0  0000000400000006 R_X86_64_GLOB_DAT      0000000000000000 __gmon_start__ + 0
0000000000003fe8  0000000600000006 R_X86_64_GLOB_DAT      0000000000000000 a + 0
0000000000003ff0  0000000700000006 R_X86_64_GLOB_DAT      0000000000000000 _ITM_registerTMCloneTable + 0
0000000000003ff8  0000000800000006 R_X86_64_GLOB_DAT      0000000000000000 __cxa_finalize@GLIBC_2.2.5 + 0

Relocation section '.rela.plt' at offset 0x668 contains 2 entries:
    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
0000000000003fc0  0000000300000007 R_X86_64_JUMP_SLOT     0000000000000000 printf@GLIBC_2.2.5 + 0
0000000000003fc8  0000000500000007 R_X86_64_JUMP_SLOT     0000000000000000 non_existing + 0
```

The [Linker and libraries Guide](https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/chapter2-11/index.html)
(which covers this topic in more detail in an older version [3]) explains that if
a weak symbol cannot be resolved, the reference is bound to address of zero. In
the example above, the value fetched from the GOT entry should be zero,
effectively a null pointer. This behavior was confirmed by examining the runtime
value of `%rax` in the instruction `mov 0x2e70(%rip),%rax`, using glibc's
dynamic linker.

For a static executable, it compile but fail to run, directly setting the
address to zero.

```bash
0000000000401745 <main>:
  401745:       f3 0f 1e fa             endbr64 
  401749:       55                      push   %rbp
  40174a:       48 89 e5                mov    %rsp,%rbp
  40174d:       48 c7 c0 00 00 00 00    mov    $0x0,%rax
  401754:       8b 00                   mov    (%rax),%eax
  401756:       89 c6                   mov    %eax,%esi
  401758:       48 8d 05 a5 68 09 00    lea    0x968a5(%rip),%rax        # 498004 <_IO_stdin_used+0x4>
  40175f:       48 89 c7                mov    %rax,%rdi
  401762:       b8 00 00 00 00          mov    $0x0,%eax
  401767:       e8 74 9e 00 00          call   40b5e0 <_IO_printf>
  40176c:       e8 8f e8 bf ff          call   0 <_nl_current_LC_CTYPE>
  401771:       b8 00 00 00 00          mov    $0x0,%eax
  401776:       5d                      pop    %rbp
  401777:       c3                      ret    
  401778:       0f 1f 84 00 00 00 00    nopl   0x0(%rax,%rax,1)
  40177f:       00
```

During linkage, it is permissible for strong symbol references remain
unresolved in a shared library, while such references result in fatal errors for
executables by default. However, linkers can ignore unresolved strong symbols
rather than reporting errors, with options such as `-z undefs` for GNU ld and
LLVM lld.

The guide [2] warns that:

> Take care when using the –z nodefs option. If an unavailable symbol reference
> is required during the execution of a process, a fatal runtime relocation
> error occurs.

Thus, one might expect that a dynamic linker should raise a fatal error if a
symbol reference is not resolved while loading executables or dynamic shared
libraries. By adopting this approach, both compile-time and runtime linking are
made more consistent. Normally, fatal errors are raised for unresolved
references to strong symbols, while unresolved weak references are permitted.
Here, I consider the loading executables and shared libraries equivalent.
However, in an experiment I conducted, a dynamic linker did not even issue an
error for referencing a strong but non-existing function as it did not notice
it.

```c
extern void non_existing(void);

int main(void) {
  non_existing();
  return 0;
}
```

The above C code compiles into the following assembly code with `-z undefs`
specified:

```bash
0000000000001149 <main>:
    1149:       f3 0f 1e fa             endbr64 
    114d:       55                      push   %rbp
    114e:       48 89 e5                mov    %rsp,%rbp
    1151:       e8 fa fe ff ff          call   1050 <__cxa_finalize@plt+0x10>
    1156:       b8 00 00 00 00          mov    $0x0,%eax
    115b:       5d                      pop    %rbp
    115c:       c3                      ret
```

This function is treated as a normal external function and invoked through the
PLT, but its PLT code is malformed, and the relocation information is missing.

```bash
Relocation section '.rela.dyn' at offset 0x530 contains 8 entries:
    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
0000000000003db8  0000000000000008 R_X86_64_RELATIVE                 1140
0000000000003dc0  0000000000000008 R_X86_64_RELATIVE                 1100
0000000000004008  0000000000000008 R_X86_64_RELATIVE                 4008
0000000000003fd8  0000000100000006 R_X86_64_GLOB_DAT      0000000000000000 __libc_start_main@GLIBC_2.34 + 0
0000000000003fe0  0000000200000006 R_X86_64_GLOB_DAT      0000000000000000 _ITM_deregisterTMCloneTable + 0
0000000000003fe8  0000000300000006 R_X86_64_GLOB_DAT      0000000000000000 __gmon_start__ + 0
0000000000003ff0  0000000400000006 R_X86_64_GLOB_DAT      0000000000000000 _ITM_registerTMCloneTable + 0
0000000000003ff8  0000000500000006 R_X86_64_GLOB_DAT      0000000000000000 __cxa_finalize@GLIBC_2.2.5 + 0

Relocation section '.rela.plt' at offset 0x5f0 contains 1 entries:
    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
0000000000000000  0000000000000000 R_X86_64_NONE                     0
```

Neither `objdump` nor `llvm-objdump` is able to disassemble the PLT code, but
`hexdump` provides assistance:

```bash
hexdump --skip 0x1050 --length 0x10 ./strong
0001050 0000 0000 0000 0000 0000 0000 0000 0000
0001060
```

It is no surprise that the dynamic linker remained silent about the missing
definition for the strong symbol; it was unaware of any reference to it. This is
because a dynamic linker resolves symbol references by populating the Global
Offset Table (GOT) with runtime addresses of symbol definitions, such as
pointers to functions or variable storage locations. In terms of dynamic shared
libraries, the dynamic linker on my system does encounter errors when strong
symbols are not resolved. Interestingly, GNU ld and LLVM lld can report errors
for unresolved symbol references by specifying the option `-z defs`.

```bash
$ ./main
Hello!
./main: symbol lookup error: libstrong.so: undefined symbol: non_existing
```

### Case 6: Interposition

The source code is located in the directory
[src/symbol/undefined](/elf/dynamic_linking/src/symbol/undefined).

Interposition plays a role when multiple definitions of a symbol exist. This
concept can be outlined as follows:

- During the linking process, a symbol definition in a relocatable file preempts
or supersedes other definitions from archive libraries or shared libraries.
Alternatively, if the references in relocatable files remain unresolved and
mutiple definitions exist in libraries, the first definition encountered is
given precedence. Link-time interposition is alreadly briefly discussed in Cases
1 and 3.

- At runtime, a symbol reference is bound to the first definition that the
dynamic linker encounters along its search paths. This suggests that the same
reference may be bound to different definitions, even using the same dynamic
linker, under slightly varied runtime conditions.

This case study concentrates on runtime interposition. Consider a scenario where
our executable requires an external function, and two distinct shared libraries
provide alternative implementations. The executable is linked to one of these
libraries. Ordinarily, the implementation from the linked library is utilized at
runtime. However, with glibc (other dynamic linkers may offer similar
flexibility), it is possible to execute the alternate implementation by setting
the `LD_PRELOAD` environment variable to the unlinked library.

Here is the relevant code setup:

```c
// api.h
#pragma once

extern void greet(void);

// impl1.c -> libimpl1.so
#include "api.h"

#include <stdio.h>

void greet(void) {
  printf("Hello, this is %s from %s\n", __func__, __FILE__);
}

// impl2.c -> libimpl2.so
#include "api.h"

#include <stdio.h>

void greet(void) {
  printf("Hello, this is %s from %s\n", __func__, __FILE__);
}

// main.c
// linked with libimpl1.so
#include "api.h"

int main(void) {
  greet();
  return 0;
}
```

Here is the output when running the exectable with and without `LD_PRELOAD`:

```bash
$ ./main
Hello, this is greet from impl1.c
$ LD_PRELOAD=./libapi2.so ./main
Hello, this is greet from impl2.c
```

## References

1. [System V Application Binary Interface][elf gabi]

[elf gabi]: https://www.sco.com/developers/gabi/latest/contents.html

2. [Oracle® Solaris 11.3 Linkers and Libraries Guide][solaris linker guide]

[solaris linker guide]: https://docs.oracle.com/cd/E53394_01/html/E54813/index.html

3. [Linker and Libraries Guide, 2010][linker guide 2010]

[linker guide 2010]: https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/index.html

4. [Linkers and Loaders][linkers and loaders]

[linkers and loaders]: https://dl.acm.org/doi/10.5555/519563
