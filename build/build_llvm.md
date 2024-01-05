# Notes on Building LLVM

## Debug information generated when sanitizers are enabled

As of [LLVM 17.0.6](https://github.com/llvm/llvm-project/tree/llvmorg-17.0.6),
debug information is packed into LLVM's binary files if `-DLLVM_USE_SANITIZER`
is specified for building. I happend to notice the size of built `llc` is far
larger than I expected, and it is confirmed by the result of issuing `file`
command.

```bash
$ file /opt/llvm-build/out/release/bin/llc
/opt/llvm-build/out/release/bin/llc: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 3.2.0, BuildID[xxHash]=52089af619959157, with debug_info, not stripped
```

I have created a script to build LLVM. [An earlier version](https://github.com/bvlgah/llvm-build/blob/46ab960d338bf5c2d619de61bb26bdb66668b81c/build_release.sh#L28)
of it set `CMAKE_BUILD_TYPE` as `Release` and used
`-DLLVM_USE_SANITIZER='Address;Undefined'` under the hood. It was confusing
since I did make a release build, and debug information was not enabled
explicitly (e.g. using `-g`).

I also found that with sanitizers specified,

1. it takes more CPU and RAM (therefore longer) which at first I thought may
result from the fact that debug information needs to be relocated during linking
an executable or shared object file as well, and

2. binary files of a debug build occupies way more space of the file system
which I believed was largely debug information.

By contrast, if none of sanitizers is specified,
there is no debug information. Therefore, I searched `CMakeLists.txt` for LLVM's
handling `LLVM_USE_SANITIZER` by

```bash
$ find /opt/llvm-project/llvm -name CMakeLists.txt | xargs rg -C 5 LLVM_USE_SANITIZER
```

And no clues. However, the compilation database created from Ninja was helpful.

```bash
$ ninja -C /opt/llvm-build/out/release -t compdb > /opt/llvm-build/out/release/compile_commands.json
$ rg ' -g' /opt/llvm-build/out/release/compile_commands.json
```

As `compile_commands.json` turned out, the compiler flag `-gline-tables-only`
(see [Clang's user manual](https://clang.llvm.org/docs/UsersManual.html#cmdoption-gline-tables-only))
was added, so information mapping binary assembly code to source file names and
line number was embedded in the file. The flag was added by CMake macro
[append_common_sanitizer_flags](https://github.com/llvm/llvm-project/blob/llvmorg-17.0.6/llvm/cmake/modules/HandleLLVMOptions.cmake#L862)
in the aftermath of `LLVM_USE_SANITIZER` being specified.

Besides, I was also curious about how much file size the debug information took.
It can be computed by comparing the original file size with that of the derived
file debug information of which is stripped by

```bash
$ llvm-strip --strip-debug -o /tmp/llc /opt/llvm-build/out/release/bin/llc
```

The original file size was 455 MiB versus 300 MiB of the stripped, and the debug
information took much less space than I expected. And I made a release build
without sanitizers, the file size of `llc` was 126 MiB. It seems
[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) (ASan)
and [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
(UBSan) make generated code larger. These two sanitizers' influence on code size
can be estimated by compiling the same source code with different compiler
flags. Taking [my solution](https://github.com/bvlgah/leetcode-solution/blob/main/problem_76_minimum_window_substring/Solution.cpp)
to [LeetCode Problem 76. Minimum Window Substring](https://leetcode.com/problems/minimum-window-substring/)
as an example, I have compiled the code by

```bash
$ clang++ -std=c++20 -O2 ${extra_cxx_flags[@]} -o Solution Solution.cpp
```

`extra_cxx_flags` and file size of binaries are as follows:

| ASan enabled | UBSan enable | extra_cxx_flags =                             | File size |
| ------------ | ------------ | --------------------------------------------- | --------- |
| No           | No           | ()                                            | 34 KiB    |
| No           | Yes          | ('-fsanitize=undefined')                      | 53 KiB    |
| Yes          | No           | ('-fsanitize=address')                        | 53 KiB    |
| Yes          | Yes          | ('-fsanitize=address' '-fsanitize=undefined') | 72 KiB    |

Conclusion:

- As for compling LLVM, line number tables (a sort of debug information) will be
generated if sanitizers are enabled (`LLVM_USE_SANITIZER`).

- With both ASan and UBSan enabled, code size is 2~3 times larger.
