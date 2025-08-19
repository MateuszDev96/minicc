#!/bin/bash

./src/minicc ./test.c > test.s
riscv64-linux-gnu-gcc -static -o test test.s
qemu-riscv64 ./test
echo "Exit code: $?"