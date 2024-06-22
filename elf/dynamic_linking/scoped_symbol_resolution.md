# ELF: An Introduction to Runtime Symbol Resolution

## 1. Overview

At first glance, runtime symbol resolution may appear straightforward, involving
the dynamic linker locating definitions within certain dependency libraries. 
However, the process is considered complex with the ELF format:

1. During runtime, if multiple symbol definitions with the same name exist
across different shared libraries, the binding of a reference to a particular
definition is uncertain. Typically, the first definition encountered in the
default search order (explored below) is chosen, while this order can be
altered using the `LD_PRELOAD` environment variable.

2. In a shared library, `DT_SYMBOLIC` (deprecated) or `DF_SYMBOLIC` [7] can be
utilized to instruct the dynamic linker to initially search within the library
itself for a definition. If unsuccessful, the default search order is then
applied.

3. Dynamic linkers may offer programming interfaces, such as `dlopen` and
`dlsym`, which allow for on-demand library loading and more fine grained control
over symbol resolution.

This article aims to providide an introductory exploration of runtime symbol
resolution in ELF and the enhanced control obtained by the runtime linking
programming interfaces.

## 2. Runtime Symbol Resolution

The focus of this discussion is on glibc's dynamic linker, with supplementary
insights from Solaris's dynamic linker. The early ELF dynamic linker likely
first appeared on Solaris 2.0, as ELF being a joint development effort of AT&T
Unix System Laboratories and Sun Microsystems [13, 14, 15, 16]. Furthermore, the
influence of Solaris is evident in the implementation of glibc's dynamic linker,
as with as its other components. For instance, several dynamic linking programming
interfaces provided by glibc, such as `dlmopen` and `dlinfo`, closely resemble
those available on Solaris. Additionally, references to Solaris' interfaces in
the man pages of glibc [2, 17, 18] underline this connection. Lastly, ELF's
default runtime symbol lookup model, which forms the foundation of symbol
resolution at runtime, can be further extended by these dynamic linking
programming interfaces.

### 2.1 Default Symbol Lookup Model

The default symbol lookup model is briefly noted in the section *Dynamic
Linking* of [12]:

> When the dynamic linker creates the memory segments for an object file, the
> dependencies (recorded in DT\_NEEDED entries of the dynamic structure) tell
> what shared objects are needed to supply the program's services. By repeatedly
> connecting referenced shared objects and their dependencies, the dynamic
> linker builds a complete process image. When resolving symbolic references,
> the dynamic linker examines the symbol tables with a breadth-first search.
> That is, it first looks at the symbol table of the executable program itself,
> then at the symbol tables of the DT\_NEEDED entries (in order), and then at
> the second level DT\_NEEDED entries, and so on. Shared object files must be
> readable by the process; other permissions are not required.

A directed graph can be contructed from an executable and all its dependencies
with `DT_NEEDED` entries as directed edges, a breadth-frst search from the
executable yields the default search order of symbol definitions. Such an order
may be counter-intuitive: a symbol referenced by a shared library should be
resolved starting from the executable, isn't it more approriate to start the
searching from the library's own dependencies? In fact, ELF dynamic linking was
initially designed to bind all references of a symbol to one defining location
(see Chapter 7, *Direct Bindings* of [1]), and permits symbol interposition
through which a symbol definition may be overrided or shadowed by another. Here
is an example of symbol interposition from the section *Runtime Linking
Programming Interface* of [1]:

```c
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *malloc(size_t size) {
  static void *(*fptr)(size_t) = NULL;
  char buffer[50];

  if (fptr == NULL) {
    fptr = (void *(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
    if (fptr == NULL) {
      (void) printf("dlopen: %s\n", dlerror());
      return (NULL);
    }
  }

  (void) sprintf(buffer, "malloc: %zu bytes\n", size);
  (void) write(1, buffer, strlen(buffer));
  return ((*fptr)(size));
}
```

Above, the function `malloc` implements the wrapper pattern, delegating memory
allocation to 'real' `malloc` and logging how many bytes of memory are
allocated. The 'real' `malloc` is located by invoking `dlsym` with `RTLD_NEXT`
(convered later).

To ensure the interposition, the shared library (e.g. `libmalloc.so.1`)
containing the wrapper function should precede libc, where the 'real' `malloc`
usually resides, in the default search order. This can be achieved by having the
library before libc in the `DT_NEEDED` entries of the executable:

```bash
$ cc -o libmalloc.so.1 -shared -fPIC malloc.c
$ cc -L. -o prog file1.o file2.o .... -lmalloc
$ prog
malloc: 0x32 bytes
malloc: 0x14 bytes
....
```

The ordering of `DT_NEEDED` entires can be confirmed by examining the
executable's `.dynamic` section:

```bash
root@omnios:~/projects/compiler-notes/elf/dynamic_linking/src/dlopen# greadelf -d ./main

Dynamic section at offset 0x1578 contains 37 entries:
  Tag        Type                         Name/Value
 0x0000000000000001 (NEEDED)             Shared library: [libmalloc.so.1]
 0x0000000000000001 (NEEDED)             Shared library: [libutil.so]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.1]
 0x000000000000000c (INIT)               0x401430
 0x000000000000000d (FINI)               0x401440
```

Alternatively, the interposition can be carried out by setting up the
environment variable `LD_PRELOAD`:

```bash
$ cc -o libmalloc.so.1 -shared -fPIC malloc.c
$ cc -o prog main.c
$ LD_PRELOAD=./libmalloc.so.1 prog
malloc: 0x32 bytes
malloc: 0x14 bytes
....
```

Nevertheless, this dynamic feature may incur considerable performance overhead.
Suppose a function is exported by a shared library, even accesses from within
the library itself are through the Procedure Linkage Table (PLT) or the Global
Offset Table (GOT). Such overhead may be significant if a library's public APIs,
like those of libpython, are heavily utilized internally. Fedora reported
performance gains of 5% to 27% on dynamically linked Python 3.8.0 built with the
`-fno-semantic-interposition` compiler option [11], comparable to those from
Python being statically linked [20]. These improvements, welcomed by the CPython
development community [10], were incorporated into Python 3.10 [21, 22]. These
gains are primarily due to more aggressive inlining (thus, more optimizatio
opportunities) and the elimination of indirections (e.g., through the GOT or the
PLT) as outlined in [19]. As a result, in the case of libpython with
`-fno-semantic-interposition`, invocations of Python APIs within the library
will not be interposed. This compiler option could therefore disrupt use cases
of `LD_PRELOAD` intended to intercept global function calls and variable
accesses **within a library defining these symbols**, hereafter referred to as
internal accesses. However, such use cases are rare and fragile, for the
following reasons:

- There are mechanisms that enforce or prefer symbol references to be resolved
within specific shared libraries, effectively circumventing symbol
interposition. For instance, the presence of `DF_SYMBOLIC` (generated with the
`-Bsymbolic` compiler option) in an ELF shared library constrains internal
accesses to be bound within the same library, which can be finalized by the
static linker as an optimization [8, 9] rather than being relocated by the
dynamic linker. Similarly, the protected visibility attribute (`STV_PROTECTED`)
restricts symbol references from within the defining library to be resolved
internally. Notably, there are peculiarities associated with the uses of
protected visibility, such as copy relocation [24, 25, 27, 28] and the
uniqueness of function pointers [8, 29, 30]. Moreover, direct bindings are
provided by Solaris' link-editor and dynamic linker to designate a symbol
definition to a specific named dependency. Unlike `-Bsymbolic`, intentional
symbol interposition is still permissible by either deactivating direct bindings
or applying interposing libraries (e.g., using the environment variable
`LD_PRELOAD` or creating a shared library as an interposer with the
`-z interpose` linker option). However, directing bindings, along with other
complex dynamic features existing on Solaris, are not supported by Linux and the
BSD family, probably due to their intricacy. For readers interested in direct
bindings, please refer to Chapter 7, *Direct Bindings* of [1].

- Symbol versioning [23] is essentially to attach version information to symbol
definitions and references, allowing for multiple versions of a symbol
definition in a single shared library. Consequently, an executable built on an
older Linux distribution can run on newer versions of the OS while its
dependency libraries evolve and maintain different versions of definitions,
which tremendously improves backward compatibility and reduces the maintenance
burden for library developers. As a side effect, symbol versioning hinders
interposition since the library name is part of the version information required
by the dynamic linker during relocation.

- Explicit uses of local aliases or wrapper functions render interposing
internal accesses impossible, as illustrated by a practical example shared by
one CPython developer [10].

Note that runtime symbol interposition is not necessarily disabled, except for
internal accesses. However, there are no guarantees regarding the use of such
interposition as it is generally not considered part of a library's API (it is a
hack, as seen in the case of libpython [10]).

### 2.2 Dynamic Linking Programming Interfaces

In this section, we will explore five of glibc's dynamic linking APIs in
principle. The discussion includes a detailed examination of glibc's
implementation, interspersed with concrete (though contrived) examples:

- `dlopen` and `dlmopen` are utilized to load shared libraries from specific
paths.

- `dlsym` and `dlvsym` are used to search a given collection of symbol tables
for a specific symbol.

- `dlclose` is designed to unload libraries that were introduced by `dlopen` and
`dlmopen`.

#### 2.2.1 Loading Libraries

The default search order establishes an **ordered** list of an executable and
its dependencies, which is referred to as the global symbol search scope. This
scope is visible to all implicit dynamic relocations (excluding those done by
`dlsym`), but with several exceptions such as `DF_SYMBOLIC` and `STV_PROTECTED`.
Additionally, the global scope can be expanded by utilizing `dlopen` with
`RTLD_GLOBAL`, where the new libraries are appended to the end of the list. The
following two figures illustrates this global scope before and after a call to
`dlopen`.

<div align="center">

![global search scope before dlopen](/elf/dynamic_linking/img/global_scope_before.svg)

Figure 1: Global search scope instantly after process initialization.

![global search scope after dlopen](/elf/dynamic_linking/img/global_scope_after.svg)

Figure 2: Global search scope after `libF.sp` is loaded with `dlopen` and
`RTLD_GLOBAL`.

</div>

With `RTLD_GLOBAL`, symbol definitions within the loaded libraries become
visible to all existing and future libraries. Furthermore, the ordering of
library additions will be honored when the global search scope is searched for
definitions. In glibc, the global scope is maintained in the field
`_ns_main_searchlist` of the struct [link\_namespaces](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/sysdeps/generic/ldsodefs.h#L315-L355),
as updated in the function [add\_to\_global\_update](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-open.c#L172-L207).

Conversely, the `RTLD_LOCAL` flag results in an absent library loaded into  a
search scope separated from the global one. Here, the contained symbol
definitions can only be accessed through the handle returned from a successful
call to `dlopen`, as depicted by Figure 3. Moreover, the obtained handle
represents not only the desired shared library, but also the symbol tables of
the library and its **dependencies**. This detail is not explicitly stated in
glibc's man page but is mentioned in the POSIX standard [5].

<div align="center">

![create non-global scope](/elf/dynamic_linking/img/add_scope.svg)

Figure 3: A new search scope is introduced as `libI.so` is loaded with
`RTLD_LOCAL`.

</div>

Two related aspects of `RTLD_LOCAL` are worth noting:

- `dlopen` can be invoked repeatedly with the same library path, ensuring only
one copy of the shared library being loaded. Given `RTLD_LOCAL` and a path
of an already installed library, the visibility of the shared library and its
dependencies will be not modified if they are globally visible.

- With `RTLD_GLOBAL`, the visibility of an already loaded library may be
promoted from local to global, along with all its dependencies. This alignment
with Solaris's dynamic linker (see Section *Runtime Linking Programming
Interface* of [1]) is demonstrated by Figure 4. On one hand, it seems reasonable
for the dependency libraries to be globally visible; otherwise, symbol
definitions within these libraries remain hidden while the search scope is
exposed. On the other hand, it is crucial to be aware that the `RTLD_LOCAL` flag
does not guarantee that a search scope will always remain local. A concrete
example of this promotion with `RTLD_GLOBAL` is located in
[src/dlopen/dlopen\_promotion.c](/elf/dynamic_linking/src/dlopen/dlopen_promotion.c).

<div align="center">

![create non-global scope](/elf/dynamic_linking/img/premote_to_global.svg)

Figure 4: The visibility of `libK.so` and `libL.so` are premoted to global after
`libK.so` is loaded again with `RTLD_GLOBAL`.

</div>

The semantics of other flags pertaining to `dlopen` include:

- The flags `RTLD_LAZY` and `RTLD_NOW` are mutually exclusive, just as
`RTLD_GLOBAL` and `RTLD_LOCAL` are, and determine whether symbol references in a
shared library are resolved before the loading is completed. Refer to my post
about [lazy loading](https://github.com/bvlgah/compiler-notes/blob/main/elf/dynamic_linking/from_execve_to_main.md#323-lazy-binding-to-functions)
for more details.

- In addition, the flags `RTLD_NODELETE` and `RTLD_NOLOAD` specify that the
library can be unloaded later and no library loaded respectively, as suggested
by their names.

- Lastly, the flag `RTLD_DEEPBIND` will be discussed in the next section.

Additionally, the handle representing the global search scope can be acquired by
passing `NULL` as the library path to `dlopen` and `dlmopen`. Since `RTLD_LOCAL`
offers no assurances regarding isolation among search scopes, `dlmopen` is
designed to load libraries into isolated namespaces, thus providing a much more
effective seal and allowing for multiple copies of one shared library, except
for the dynamic linker. During the initialization of a process, the executable
and its dependencies are installed into the default namespace, denoted by
`LM_ID_BASE`. Later, libraries are loaded into the namespace their caller
belongs to, except for those involved by `dlmopen` with either `LM_ID_NEWLM`
or handles to namespaces different from the callers'. Within a new namespace,
library loading works in the same manner as in the default namepsace. Moreover,
there is a long-standing bug [3] related to the use of non-default namespaces.
Specifically, the `RTLD_GLOBAL` flag cannot be used with `dlopen` if the calling
function does not belong to the default namespace; otherwise, a segment fault
will be raised. This is because the previous mentioned field
`_ns_main_searchlist` is not initialized for other namespaces than the default
one. As a result, the executable compiled from [src/dlopen/main.c](
/elf/dynamic_linking/src/dlopen/main.c) could raise an error if the command
line option `-g` is given.

#### 2.2.2 Symbol Searching with `dlsym` and `dlvsum`

As shared libraries can be loaded on-demand, one aspect not yet explored of the
default search model is: given a symbol is referenced by a dynamically loaded
library and is not resolved in the global scope, additional scopes containing
the library will be searched for the definition. Taking Figure 4 as an example,
suppose the function `foo` is utilized by `libK.so` and cannot be resolved in
the global scope, then the search scope #1 will be inspected. With regard to
`dlsym`, the default search model can be precisely performed with `RTLD_DEFAULT`
as the symbol table handle. Moreover, if a library and its dependencies are
involved in a new scope by `dlopen` and the flag `RTLD_DEEPBIND`, this scope
will be searched for definitions before the global scope.

Not depicted in the above 4 figures, every handle obtained from `dlopen` comes
with a local search scope, composed of the library directly represented by the
handle and its dependencies. For example, in Figure 4, besides introducing
`libK.so` and `libL.so` into the global scope, a handle representing the search
scope consisting of the two libraries is returned, as a result of invoking
`dlopen` with the path to `libK.so` and the flag `RTLD_GLOBAL`. Furthermore, if
the handle is utilized along with `dlsym`, then only `libK.so` and `libL.so` are
explored.

Regarding glibc's implementation, a loaded library is modeled by the struct
[link\_map](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/include/link.h#L95-L348).
For the executable and explicitly loaded libraries with `dlopen` or `dlmopen`
(paths of those passed as arguments), their field `l_searchlist` is set to their
dependencies within the function [\_dl\_map\_object\_deps](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-deps.c#L462-L491).
While instances of `libk_map` are initialized, symbol search scopes are attached
to them by setting up the field `l_scope`:

- By default, the `l_searchlist` of
[the executable](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-object.c#L157)
and [the dependent library](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-object.c#L176)
are appended in order.

- If `RTLD_DEEPBIND` is specified to `dlopen`, the positions of the `exectuable`
and the dependent in `l_scope` are [exchanged](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-object.c#L170-L176).

- If the flag `DF_SYMBOLIC` is embedded in the library, the first element of
`l_scope` is [the library itself](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-load.c#L1416-L1431).

Seemingly obscure at first, another field [l\_local\_scope](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/include/link.h#L267)
of the struct `link_map` is introduced to record local search scopes, which are
utlized by `dlsym`:

- If the pseudo handle `RTLD_NEXT` is passed to `dlsym`, the local scope of the
first involved dependent library in the timeline are searched for symbol
definitions, implemented in the function [do\_sym](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-sym.c#L129-L148).
Incidentally, [l\_loader](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/include/link.h#L151)
is a confusing name for denoting the dependent library, while I would recommend
`l_dependent`.

- When a concrete handle is used with `dlsym`, its own local scope will be
examined, shown in the function [do\_sym](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-sym.c#L149-L155)

Finally, `dlvsym` works almost the same was `dlsym`, except for additional
version strings, which is covered in [my post on symbol versioning](
https://github.com/bvlgah/compiler-notes/blob/main/elf/dynamic_linking/symbol_versioning.md).

#### 2.2.3 Removal of Libraries

If a library is considered no longer useful, we can inform the dynamic linker
that the library can be removed. However, the library may not be
unloaded immediately upon a successful call to `dlclose`, because there may be
mutliple handles to the library resulting from invoking `dlopen` multiple times.
In order to avoid fatal use-after-free errors, a refrence count is maintained
for every library dynamically loaded, incrementing on `dlopen` and decrementing
on `dlclose`. The dynamic linker does not consider the removal of a library
until its reference count is down to zero. In regard to glibc, another
requirement for the removal is that symbol definitions within the library are
not accessed by other modules in the process. In addition to the relations
conveyed explcitly by `DT_NEEDED` entries, glibc also keeps symbol dependencies
when symbol references are resolved during relocation and explicit lookup by
`dlsym` or `dlvsym`. Thus, if there is a symbol definition accessed by other
libraries, the containing library will not be removed even if the reference
count reduces down to zero, as it is resurrected by the dynamic linker within
the function [\_dl\_close\_worker](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/dl-close.c#L220-L236).
Refer to [src/dlopen/dlclose.c](/elf/dynamic_linking/src/dlopen/dlclose.c) for
a tangible example.

Given that another popular libc implementation, musl (as of June 2024), [does
not supported library unloading](https://git.musl-libc.org/cgit/musl/tree/src/ldso/dlclose.c),
I believe there is no definitely reliable method to remove a dynamically loaded
library.

## References

[1] [Oracle® Solaris 11.3 Linkers and Libraries Guide][linkers guide]

[2] [dlopen(3) — Linux manual page][glibc dlopen man page]

[3] [glibc - Bug 18684 - dlmopen a DSO that dlopen's into RTLD_GLOBAL segfaults][glibc bug 18684]

[4] [dlmopen(3C) — Solaris man pages][solaris dlmopen man page]

[5] [dlopen - The Open Group Base Specifications Issue 7, 2018 edition][posix
2018 dlopen]

[6] [Glibc Source Code][glibc source code]

[7] [Tool Interface Standard (TIS) Portable Formats Specification, version 1.2][
elf spec 1.2]

[8] Fangrui Song's [ELF interposition and -Bsymbolic][elf interposition and
symbolic]

[9] Fangrui Song's [-fno-semantic-interposition][disable interposition]

[10] [CPython - Issue 38980 - Compile libpython with -fno-semantic-interposition][libpython no semantic interposition]

[11] [Fedora - Changes/PythonNoSemanticInterpositionSpeedup][fedora report on
python performance gain]

[12] [System V Application Binary Interface - DRAFT - 10 June 2013][gabi latest
draft]

[13] [OSDev Wiki - ELF][osdev wiki elf]

[14] [Wikipedia - Executable and Linkable Format][wikipedia elf]

[15] [Wikipedia - UNIX System V][wikipedia unix system v]

[16] Fangrui Song's [Evolution of the ELF object file format][evolution of elf]

[17] [dlsym(3) — Linux manual page][glibc dlsym man page]

[18] [dlinfo(3) — Linux manual page][glibc dlinfo man page]

[19] [Red Hat Enterprise Linux 8.2 brings faster Python 3.8 run speeds][red hat
python speedup]

[20] [Fedora - Changes/PythonStaticSpeedup][fedora report on static python]

[21] [CPython - PR 22862][cpython pr22862]

[22] [CPython - PR 22892][cpython pr22892]

[23] [Linux Standard Base Core Specification, Generic Part, 5.0 Edition][lsb
core generic]

[24] [Oracle® Solaris Linkers and Libraries Guide, 2010][linkers guide 2010]

[25] Fangrui Song's [Copy relocations, canonical PLT entries and protected
visibility][fangrui on protected visibility]

[26] [GCC - Bug 55012 - Protected visibility wrongly uses GOT-relative addresses][gcc bug 55012]

[27] [GCC - Bug 65248 - Copy relocation against protected symbol doesn't work][gcc bug 65248]

[28] [GCC mailing list - \[x86-64\] Default HAVE\_LD\_PIE\_COPYRELOC to false][gcc rfc copy relocation]

[29] [GCC - Bug 98112 - Add -f\[no-\]direct-access-external-data & drop HAVE\_LD\_PIE\_COPYRELOC][gcc bug 98112]

[30] [GCC - Bug 100593 - \[ELF\] -fno-pic: Use GOT to take address of an external default visibility function][gcc bug 100593]

[linkers guide]: https://docs.oracle.com/cd/E53394_01/html/E54813/index.html

[glibc dlopen man page]: https://www.man7.org/linux/man-pages/man3/dlopen.3.html

[glibc bug 18684]: https://sourceware.org/bugzilla/show_bug.cgi?id=18684

[solaris dlmopen man page]: https://docs.oracle.com/cd/E86824_01/html/E54766/dlmopen-3c.html

[posix 2018 dlopen]: https://pubs.opengroup.org/onlinepubs/9699919799/

[glibc source code]: https://sourceware.org/git/?p=glibc.git

[elf spec 1.2]: https://refspecs.linuxfoundation.org/elf/elf.pdf

[elf interposition and symbolic]: https://maskray.me/blog/2021-05-16-elf-interposition-and-bsymbolic

[disable interposition]: https://maskray.me/blog/2021-05-09-fno-semantic-interposition

[libpython no semantic interposition]: https://bugs.python.org/issue38980

[fedora report on python performance gain]: https://fedoraproject.org/wiki/Changes/PythonNoSemanticInterpositionSpeedup

[gabi latest draft]: https://www.sco.com/developers/gabi/latest/contents.html

[osdev wiki elf]: https://wiki.osdev.org/ELF

[wikipedia elf]: https://en.wikipedia.org/wiki/Executable_and_Linkable_Format

[wikipedia unix system v]: https://en.wikipedia.org/wiki/UNIX_System_V

[evolution of elf]: https://maskray.me/blog/2024-05-26-evolution-of-elf-object-file-format

[glibc dlsym man page]: https://www.man7.org/linux/man-pages/man3/dlsym.3.html

[glibc dlinfo man page]: https://www.man7.org/linux/man-pages/man3/dlinfo.3.html

[red hat python speedup]: https://developers.redhat.com/blog/2020/06/25/red-hat-enterprise-linux-8-2-brings-faster-python-3-8-run-speeds

[fedora report on static python]: https://fedoraproject.org/wiki/Changes/PythonStaticSpeedup

[cpython pr22862]: https://github.com/python/cpython/pull/22862

[cpython pr22892]: https://github.com/python/cpython/pull/22892

[lsb core generic]: https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/book1.html

[linkers guide 2010]: https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/index.html

[fangrui on protected visibility]: https://maskray.me/blog/2021-01-09-copy-relocations-canonical-plt-entries-and-protected

[gcc bug 55012]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55012

[gcc bug 65248]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65248

[gcc rfc copy relocation]: https://gcc.gnu.org/legacy-ml/gcc/2019-05/msg00215.html

[gcc bug 98112]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98112

[gcc bug 100593]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100593
