# clcov: Check OpenCL Kernel code coverage

## Introduction

Based on Clang Libtooling technique, clcov is developped to insturment OpenCL kernel source code and check branch coverage of the program.

## Folder Structure

* **src** Source code

## Build

1. To build and use this tool, please first follow the instructions on how to build Clang/LLVM on your machine

    [http://clang.llvm.org/docs/LibASTMatchersTutorial.html](http://clang.llvm.org/docs/LibASTMatchersTutorial.html)

2. Clone clcov to `~/clang-llvm/llvm/tools/clang/tools/extra`

3. Add the following line to `~/clang-llvm/llvm/tools/clang/tools/extra/CMakeLists.txt`

```
    add_subdirectory(clcov)
```

4. Build clcov

```bash
    cd ~/clang-llvm/build
    ninja
```

## Usage

To run clcov on your kernel source code:

```bash
    ~/clang-llvm/llvm/build/bin/clcov yourkernelfile.cl -o outpurdirectory
```

After running this command, the instrumented kernel source code along with our profiling datasets will be written to the directory you provide.
