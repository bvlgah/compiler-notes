# Example of Using LLVM

## Running LLVM passes

First of all, an installation of LLVM 15 is needed. On Ubuntu 22.04, it can
installed by

```bash
$ sudo apt install llvm-15
```

Then, build the repo (assuming `cmake` and `ninja` are already installed) by

```bash
$ cmake -G Ninja -S . -B out
$ ninja -C out
```

Some pluggable passes are provided by `LLVMHello` (in the new pass manager
style, see [useful instructions on writing LLVM passes with the new pass
manager](https://medium.com/@mshockwave/writing-llvm-pass-in-2018-preface-6b90fa67ae82)),
these can be run on Ubuntu with

```
opt-15 -load out/llvm_code_example/LLVMHello.so \
  -disable-output \
  -passes=live-variable llvm_code_example/test/livevar.ll
```
