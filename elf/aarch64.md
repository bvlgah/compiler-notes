# ELF for ARM 64

## 1. ELF header

```C++
using Elf64_Addr = uint64_t;
using Elf64_Off = uint64_t;
using Elf64_Half = uint16_t;
using Elf64_Word = uint32_t;
using Elf64_Sword = int32_t;
using Elf64_Xword = uint64_t;
using Elf64_Sxword = int64_t;

// Object file magic string.
static const char ElfMagic[] = {0x7f, 'E', 'L', 'F', '\0'};

// e_ident size and indices.
enum {
  EI_MAG0 = 0,       // File identification index.
  EI_MAG1 = 1,       // File identification index.
  EI_MAG2 = 2,       // File identification index.
  EI_MAG3 = 3,       // File identification index.
  EI_CLASS = 4,      // File class.
  EI_DATA = 5,       // Data encoding.
  EI_VERSION = 6,    // File version.
  EI_OSABI = 7,      // OS/ABI identification.
  EI_ABIVERSION = 8, // ABI version.
  EI_PAD = 9,        // Start of padding bytes.
  EI_NIDENT = 16     // Number of bytes in e_ident.
};

struct Elf64_Ehdr {
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;
  Elf64_Off e_phoff;
  Elf64_Off e_shoff;
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
}
```

An example

```
00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00
00000010  03 00 b7 00 01 00 00 00  a4 18 00 00 00 00 00 00
00000020  40 00 00 00 00 00 00 00  98 2b 00 00 00 00 00 00
00000030  00 00 00 00 40 00 38 00  0b 00 40 00 25 00 23 00
```

### 1.1 EFL identification

The first 16 bytes of the example:

| Field     | Offset in bytes (from the start of the elf header) | Value  | Note |
| --------- | -------------------------------------------------- | ------ | ---- |
| `EI_MAG0` | 0x00 | `0x7f` |      |
| `EI_MAG1` | 0x01 | `0x45` (`'E'`) | |
| `EI_MAG2` | 0x02 | `0x4c` (`'L'`) | |
| `EI_MAG3` | 0x03 | `0x46` (`'F'`) | |
| `EI_CLASS` | 0x04 | `0x02` | 64-bit format |
| `EI_DATA` | 0x05 | `0x01` | little endian |
| `EI_VERSION` | 0x06 | `0x01` | Must be `0x01` |
| `EI_OSABI` | 0x07 | `0x00` | System V |
| `EI_ABIVERSION` | 0x08 | `0x00` | Not set |

### 1.2 Other parts

The remaining 48 bytes

| Field | Offset in bytes (from the start of the elf header) | Size in bytes | Value | Note |
| ----- | -------------------------------------------------- | ------------- | ----- | ---- |
| `e_type` | 0x10 | 2 | `0x03` | Shared object |
| `e_machine` | 0x12 | 2 | `0xb7` | ARM AArch64; 200+ known possible value representing different hardware architectures |
| `e_version` | 0x14 | 4 | `0x01` | Must be `0x01` |
| `e_entry` | 0x18 | 8 | `0x18a4` | Virtual address of the entry point |
| `e_phoff` | 0x20 | 8 | `0x40` | Offset (in bytes) of the program header |
| `e_shoff` | 0x28 | 8 | `0x2b98` | Offset (in bytes) of the section header table |
| `e_flags` | 0x30 | 4 | `0x00` | Shall be `0x00` |
| `e_ehsize` | 0x34 | 2 | `0x40` | Size (in bytes) of the elf header |
| `e_phentsize` | 0x36 | 2 | `0x38` | Size (in bytes) of an entry in the program header |
| `e_phnum` | 0x38 | 2 | `0x0b` | Number of entries in the program header |
| `e_shentsize` | 0x3a | 2 | `0x40` | Size (in bytes) of an entry in the section header table |
| `e_shnum` | 0x3c | 2 | `0x25` | Number of entries in the section header table |
| `e_shstrndx` | 0x3e | 2 | `0x23` | Index of the section header entry that contains the section name string table |

## 2. Section header

```C++
// Section header for ELF64 - same fields as ELF32, different types.
struct Elf64_Shdr {
  Elf64_Word sh_name;
  Elf64_Word sh_type;
  Elf64_Xword sh_flags;
  Elf64_Addr sh_addr;
  Elf64_Off sh_offset;
  Elf64_Xword sh_size;
  Elf64_Word sh_link;
  Elf64_Word sh_info;
  Elf64_Xword sh_addralign;
  Elf64_Xword sh_entsize;
};

// Special section indices.
enum {
  SHN_UNDEF = 0,          // Undefined, missing, irrelevant, or meaningless
  SHN_LORESERVE = 0xff00, // Lowest reserved index
  SHN_LOPROC = 0xff00,    // Lowest processor-specific index
  SHN_HIPROC = 0xff1f,    // Highest processor-specific index
  SHN_LOOS = 0xff20,      // Lowest operating system-specific index
  SHN_HIOS = 0xff3f,      // Highest operating system-specific index
  SHN_ABS = 0xfff1,       // Symbol has absolute value; does not need relocation
  SHN_COMMON = 0xfff2,    // FORTRAN COMMON or C external global variables
  SHN_XINDEX = 0xffff,    // Mark that the index is >= SHN_LORESERVE
  SHN_HIRESERVE = 0xffff  // Highest reserved index
};
```

### 2.1 Exception frames

#### 2.1.1 The `.eh_frame_hdr` section

#### 2.1.2 The `.eh_frame` section

## 3. Program header

## References

1. [ELF for the Arm® 64-bit Architecture (AArch64)](https://github.com/ARM-software/abi-aa/blob/main/aaelf64/aaelf64.rst)

2. [llvm/BinaryFormat/ELF.h](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/BinaryFormat/ELF.h)

3. [Tool Interface Standard (TIS) Executable and Linking Format (ELF) Specification](https://refspecs.linuxfoundation.org/elf/elf.pdf)
