# Symbol Versioning

Note: I am using Ubuntu 22.04.4 LTS, therefore this artile only applies to
Linux. And this article is only concerned about ELF for 64-bit systems.

Here is the section header of `/usr/lib/llvm-19/lib/libLLVM.so.19.0`:

```
Section Headers:                                                                                         
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al 
  [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0 
  [ 1] .note.gnu.build-id NOTE           0000000000000238 000238 000024 00   A  0   0  4 
  [ 2] .dynsym           DYNSYM          0000000000000260 000260 12f018 18   A  3   1  8 
  [ 3] .dynstr           STRTAB          000000000012f278 12f278 394d0d 00   A  0   0  1
  [ 4] .gnu.hash         GNU_HASH        00000000004c3f88 4c3f88 062158 00   A  2   0  8
  [ 5] .gnu.version      VERSYM          00000000005260e0 5260e0 019402 02   A  2   0  2
  [ 6] .gnu.version_d    VERDEF          000000000053f4e4 53f4e4 000038 00   A  3   2  4
  [ 7] .gnu.version_r    VERNEED         000000000053f51c 53f51c 000350 00   A  3   9  4
  [ 8] .rela.dyn         RELA            000000000053f870 53f870 73f488 18   A  2   0  8
  [ 9] .rela.plt         RELA            0000000000c7ecf8 c7ecf8 002478 18  AI  2  24  8
  [10] .init             PROGBITS        0000000000c81170 c81170 00001b 00  AX  0   0  4
  [11] .plt              PROGBITS        0000000000c81190 c81190 001860 10  AX  0   0 16
  [12] .text             PROGBITS        0000000000c82a00 c82a00 3705032 00  AX  0   0 64
  [13] .fini             PROGBITS        0000000004387a34 4387a34 00000d 00  AX  0   0  4
  [14] .rodata           PROGBITS        0000000004387a80 4387a80 289c11c 00   A  0   0 64 
  [15] .eh_frame         X86_64_UNWIND   0000000006c23ba0 6c23ba0 5d541c 00   A  0   0  8
  [16] .eh_frame_hdr     X86_64_UNWIND   00000000071f8fbc 71f8fbc 0e2cbc 00   A  0   0  4     
  [17] .tdata            PROGBITS        00000000072dcc80 72dbc80 000004 00 WAT  0   0  4    
  [18] .tbss             NOBITS          00000000072dcc88 72dbc84 000028 00 WAT  0   0  8 
  [19] .fini_array       FINI_ARRAY      00000000072dcc88 72dbc88 000008 08  WA  0   0  8
  [20] .init_array       INIT_ARRAY      00000000072dcc90 72dbc90 001480 08  WA  0   0  8 
  [21] .data.rel.ro      PROGBITS        00000000072de110 72dd110 507570 00  WA  0   0 16 
  [22] .dynamic          DYNAMIC         00000000077e5680 77e4680 0002c0 10  WA  3   0  8 
  [23] .got              PROGBITS        00000000077e5940 77e4940 0076a8 00  WA  0   0  8  
  [24] .got.plt          PROGBITS        00000000077ecfe8 77ebfe8 000c40 00  WA  0   0  8
  [25] .tm_clone_table   PROGBITS        00000000077edc28 77ecc28 000000 00  WA  0   0  8
  [26] .data             PROGBITS        00000000077edc30 77ecc30 049640 00  WA  0   0 16
  [27] .bss              NOBITS          0000000007837270 7836270 09a1f9 00  WA  0   0 16
  [28] .note.gnu.gold-version NOTE       0000000000000000 7836270 00001c 00      0   0  4                                                                                                                   
  [29] .gnu_debuglink    PROGBITS        0000000000000000 783628c 000034 00      0   0  4               
  [30] .shstrtab         STRTAB          0000000000000000 78362c0 000133 00      0   0  1
```

I was curious about the purpose of three sections `.gnu.version`,
`.gnu.version_r` and `.gnu.version_d`. It seems these are related to symbol
versioning. Besides, I have encountered an issue about versioning that there is
an error complaining about the uncompatible version of glibc while running on
Ubuntu 18.04 LLVM's binary files built on Ubuntu 20.04. These two factors have
been pushing me to figure out how symbol versioning works in ELF.

## Symbol versioning at a glance

DJ Delorie wrote an introductory article, [How the GNU C Library handles backward compatibility][
dj delorie glibc back compat], about symbol versioning. It can be summarized as
the following:

- Multiple verions of a symbol may exist in a shared object. For example, there
are `glob64@GLIBC_2.1`, `glob64@GLIBC_2.2` and `glob64@@GLIBC_2.27` in glibc.

- During linking, the linker binds a symbol reference to its default version (
indicated by `@@`). In the example above, it is `glob64@@GLIBC_2.27`.

- At runtime, the dynamic linker searches for the specific version of a symbol.

My first thought was that a whole versioned symbol could be recorded in the
dynamic symbol table. So I checked the content of `libLLVM.so.19.0`'s `.dynsym`. The
result of `llvm-readel` shows verioned symbols, e.g.

```
Symbol table '.dynsym' contains 51713 entries:
   Num:    Value          Size Type    Bind   Vis       Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT   UND
     1: 0000000000000000     0 FUNC    GLOBAL DEFAULT   UND _ZSt20__throw_out_of_rangePKc@GLIBCXX_3.4
...
```

If so, why are there three sections prefixed with `.gnu.version`? It is possible
`llvm-readelf` has made some magic. I believe that it is necessary to examine
the content of `.dynsym` and `dynstr` manually.

```
typedef struct
{
  Elf64_Word    st_name;    /* Symbol name (string tbl index) */
  unsigned char st_info;    /* Symbol type and binding */
  unsigned char st_other;   /* Symbol visibility */
  Elf64_Section st_shndx;   /* Section index */
  Elf64_Addr    st_value;   /* Symbol value */
  Elf64_Xword   st_size;    /* Symbol size */
} Elf64_Sym;
```

A symbol table (`.dynsym` or `.symtab`) of a 64-bit ELF file has entries of
type [Elf64_Sym](
https://github.com/bminor/glibc/blob/8f58e412b1e26d2c7e65c13a0ce758fbaf18d83f/elf/elf.h#L530)
occupying 24 bytes. According to [ELF spec v1.2][elf spec v1.2],
fields of the first entry (with index 0 and with offset `0x260` in this case) is
set with value 0. Let's start with the second entry (`0x260 + 0x18`) of section
`.dynsym` which has offset `0x260` from the begining of file `libLLVM.so.19.0`.

```
$ hexdump --skip 0x278 --length 24 -C /usr/lib/llvm-19/lib/libLLVM.so.19.0
00000278  33 00 00 00 12 00 00 00  00 00 00 00 00 00 00 00  |3...............|
00000288  00 00 00 00 00 00 00 00                           |........|
00000290
```

The fields of the second entry have values as follows:

```
st_name = 0x33
st_info = 0x12
st_other = 0x0
st_shndx = 0x0
st_value = 0x0
st_size = 0x0
```

This symbol is a global (`st_info & 0xf = 0x2`) function (`st_info >> 4 = 0x1`).
With `st_value` being `0x0`, it is a symbol referenced by `libLLVM.so.19.0` and
located in another shared object.

`.dynstr` is at offset `0x12f278`, the corresponding symbol name (with offset
`0x12f278 + 0x33 = 0x12f2ab`) is `_ZSt20__throw_out_of_rangePKc`.

```
$ hexdump --skip 0x12f2ab --length 64 -C /usr/lib/llvm-19/lib/libLLVM.so.19.0
0012f2ab  5f 5a 53 74 32 30 5f 5f  74 68 72 6f 77 5f 6f 75  |_ZSt20__throw_ou|
0012f2bb  74 5f 6f 66 5f 72 61 6e  67 65 50 4b 63 00 47 4c  |t_of_rangePKc.GL|
0012f2cb  49 42 43 58 58 5f 33 2e  34 00 6c 69 62 73 74 64  |IBCXX_3.4.libstd|
0012f2db  63 2b 2b 2e 73 6f 2e 36  00 74 6f 75 70 70 65 72  |c++.so.6.toupper|
0012f2eb
```

A string in ELF is a null-terminated character sequence like C. In our example,
the symbol name is `_ZSt20__throw_out_of_rangePKc` which does not carry the 
version directly. It happens to be in accordance with what is mentioned in [the
user guide to the GNU assembler as, 7.97 .symver](
https://sourceware.org/binutils/docs/as/Symver.html),

> For ELF targets, the .symver directive can be used like this:
>
> .symver name, name2@nodename[ ,visibility ]
>
> If the original symbol name is defined within the file being assembled, the
> .symver directive effectively creates a symbol alias with the name
> name2@nodename, and in fact the main reason that we just don’t try and create
> a regular alias is that the @ character isn’t permitted in symbol names.

## Symbol version table

As shown above, symbol names in `.dynstr` do not have versions associated. And
it turns out version information is encoded in three sections `.gnu.version`,
`.gnu.version_d` and `.gnu.version_r`.

### .gnu.version

Section `.gnu.version` contains an array of identifiers which can be decoded
into version information along with `.gnu.version_d` (for symbols defined by
the current shared object) or `.gnu.version_r` (for those required). The
adddress of this section is referenced by an entry of type [DT_VERSYM](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/elf.h#L969)
in section `.dynamic`. Such an identifier is of type [Elf64_Versym](https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/elf.h#L56),
and it takes up 2 bytes.

```
typedef Elf64_Half Elf64_Versym;
```

One of important things to note is that the static linked should ignore the
symbol if the highest bit (bit 15) of its identifier is set, according to [Linux
Standard Base Core Specification 5.0, Generic Part, 10.7.6. Symbol Resolution][
lsb 5.0 10.7.6 symbol resolution]. As we shall, this feature is used to
implement the default version of symbol exported by a shared object.

There two versions of `glob64` in glibc built by myself:

```
Symbol table '.dynsym' contains 3157 entries:
   Num:    Value          Size Type    Bind   Vis       Ndx Name
...
  1868: 000000000014b200  6604 FUNC    GLOBAL DEFAULT    16 glob64@GLIBC_2.2.5
...
  1871: 00000000000d2510  6604 FUNC    GLOBAL DEFAULT    16 glob64@@GLIBC_2.27
...
```

With `.gnu.version` having offset `0x02051a`, the offset for
`glob64@GLIBC_2.2.5` is `0x02051a + 1868 * 2 = 0x213b2`, and `0x02051a + 1871 *
2 = 0x213b8`.

```
$ hexdump --skip 0x213b2 --length 16 -C ../install/lib/libc.so.6
000213b2  02 80 23 00 1d 80 1c 00  02 00 02 00 02 00 29 00  |..#...........).|
000213c2
```

The identifier to `glob64@GLIBC_2.2.5` is `0x8002` with the highest bit set
while the one to `glob64@@GLIBC_2.27` is `0x001c`.

### .gnu.verion_d

Section `.gnu.version_d` is used to store version information of symbols defined
by the current shared object. An instance of [Elf64_Verdef](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/elf.h#L1056-L1066)
associated with one or multiple instances of [Elf64_Verdaux](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/elf.h#L1094-L1099)
is used to model a version of a specific symbol. `vd_ndx` of `Elf64_Verdef`
provides the value to identifiers in `.gnu.version`. It seems `Elf64_Verdef` and
`Elf64_Verdaux` can be merged with `vda_next` omitted assuming there is only one
verdaux array. However, it is not uncommon for a version definition to own
multiple such arraies. The symbol name is always provided by the first array
while the real purpose of others remain unknown. Both GNU binutils and [LLVM](
https://github.com/llvm/llvm-project/blob/83646590afe222cfdd792514854549077e17b005/llvm/tools/llvm-readobj/ELFDumper.cpp#L7443-L7466)
interpret the second verdaux array as the dependecy of the first one.
[GCC](https://gcc.gnu.org/wiki/SymbolVersioning) and [GNU ld](
https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_25.html) also mention
it. It appeas glibc's dynamic linker (as of commit
`6d3b523eb54198d15af6e042054912cf9f5210de`) does ignore verdaux arrays other
than the first one as there is no use of `vda_next` throughout the entire
codebase.

```
typedef struct
{
  Elf64_Half  vd_version;   /* Version revision */
  Elf64_Half  vd_flags;     /* Version information */
  Elf64_Half  vd_ndx;       /* Version Index */
  Elf64_Half  vd_cnt;       /* Number of associated aux entries */
  Elf64_Word  vd_hash;      /* Version name hash value */
  Elf64_Word  vd_aux;       /* Offset in bytes to verdaux array */
  Elf64_Word  vd_next;      /* Offset in bytes to next verdef
                               entry */
} Elf64_Verdef;

typedef struct
{
  Elf64_Word  vda_name;     /* Version or dependency names */
  Elf64_Word  vda_next;     /* Offset in bytes to next verdaux
                               entry */
} Elf64_Verdaux;
```

The version information of previous two versioned symbol is as follows:

```
Version definition section '.gnu.version_d' contains 41 entries:
 Addr: 0000000000021dc8  Offset: 0x021dc8  Link: 6 (.dynstr)
...
  0x001c: Rev: 1  Flags: none  Index: 2  Cnt: 1  Name: GLIBC_2.2.5
...
  0x03bc: Rev: 1  Flags: none  Index: 28  Cnt: 2  Name: GLIBC_2.27
  0x03d8: Parent 1: GLIBC_2.26
...
```

### .gnu.version_r

Version information for symbols required by the current shared object is grouped
by shared object files needed. An instance of [Elf64_Verneed](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/elf.h#L1115-L1124)
represents a required object file, and each of its assocated [Elf64_Vernaux](
https://github.com/bminor/glibc/blob/caed1f5c0b2e31b5f4e0f21fea4b2c9ecd3b5b30/elf/elf.h#L1144-L1152)
array holds one piece of version information. In my opinion, this way of
organizing required symbols' version information can help version matching by
comparing entries of the calling object's `Elf64_Verneedr` and entries of the
dependent object's `Elf64_Verdef` as mentioned by [Linux Standard Base Core
Specification 5.0, Generic Part, 10.7.5. Startup Sequence][
lsb 5.0 10.7.5 start sequence].

```
typedef struct
{
  Elf64_Half  vn_version;   /* Version of structure */
  Elf64_Half  vn_cnt;       /* Number of associated aux entries */
  Elf64_Word  vn_file;      /* Offset of filename for this
                               dependency */
  Elf64_Word  vn_aux;       /* Offset in bytes to vernaux array */
  Elf64_Word  vn_next;      /* Offset in bytes to next verneed
                               entry */
} Elf64_Verneed;

typedef struct
{
  Elf64_Word  vna_hash;     /* Hash value of dependency name */
  Elf64_Half  vna_flags;    /* Dependency specific information */
  Elf64_Half  vna_other;    /* Version Index */
  Elf64_Word  vna_name;     /* Dependency name string offset */
  Elf64_Word  vna_next;     /* Offset in bytes to next vernaux
                               entry */
} Elf64_Vernaux;
```

```
Version needs section '.gnu.version_r' contains 1 entries:
 Addr: 0000000000022378  Offset: 0x022378  Link: 6 (.dynstr)
  0x0000: Version: 1  File: ld-linux-x86-64.so.2  Cnt: 3
  0x0010:   Name: GLIBC_2.2.5  Flags: none  Version: 44
  0x0020:   Name: GLIBC_2.3  Flags: none  Version: 43
  0x0030:   Name: GLIBC_PRIVATE  Flags: none  Version: 42
```

## Creating versioned symbols

Using GNU assembler's `.symver` directive, a versioned symbol can be created by
an inline assembly like below (from [DJ Delorie's article][dj delorie glibc back compat]):

```
__asm__(".symver lookup_v2, lookup@@v2");
int lookup_v2 (int index, void *data)
{
 . . .

}

__asm__(".symver lookup_v1, lookup@");
int lookup_v1 (int index)
{
 . . .

}
```

## Future works

1. Figure out other use cases of multiple instances of `Elf64_Verdaux` than
`readelf` and `llvm-readelf`.

2. Add more details about how a dynamic linker like glibc's matches versioned
symbols.

## References

[Tool Interface Standard (TIS) Executable and Linking Format (ELF) Specification Version 1.2][elf spec v1.2]

[elf spec v1.2]: https://refspecs.linuxfoundation.org/elf/elf.pdf

[Linux Standard Base Core Specification 5.0, Generic Part][lsb 5.0]

[lsb 5.0]: https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/book1.html
[lsb 5.0 10.7.5 start sequence]: https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/symversion.html#SYMSTARTSEQ
[lsb 5.0 10.7.6 symbol resolution]: https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/symversion.html#SYMRESOLUTION

DJ Delorie's [How the GNU C Library handles backward compatibility][dj delorie glibc back compat]

[dj delorie glibc back compat]: https://developers.redhat.com/blog/2019/08/01/how-the-gnu-c-library-handles-backward-compatibility
