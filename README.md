# Operating-System-Course-Design

## Environment

- Ubuntu 21.04
- gcc-2.8.1-mips.tar.gz
- GNU Make 4.3

## Build

e.g.

```shell
cd ./code/threads
make clean
make
```

## Overview

```
.
├── c++example
├── code
│   ├── ass3
│   ├── ass4
│   ├── bin
│   ├── filesys: implemention of original Nachos file system
│   ├── lab2: implemention of how to use makefile
│   ├── lab3: implemention of "producer and consumer problem"
│   ├── lab5: implemention of Nachos file system extension
│   ├── lab7-8: implemention of multi-process and some system calls
│   ├── lab9: implemention of some system calls and shell
│   ├── machine: simulation of hardwares
│   ├── Makefile.common
│   ├── Makefile.dep
│   ├── monitor
│   ├── network
│   ├── test: sources of some user programs
│   ├── threads: management of kernel threads
│   ├── userprog: implemention of how to execute user programs
│   └── vm
├── gcc-2.8.1-mips.tar.gz
└── README.md
```
