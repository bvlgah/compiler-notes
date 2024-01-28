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

Some pluggable passes are provided by `LLVMHello`, these can be run on Ubuntu
with

```
opt-15 --load out/llvm_code_example/LLVMHello.so --enable-new-pm=0 \
  --live-variable -S < test/livevar.ll 1> /dev/null
```
