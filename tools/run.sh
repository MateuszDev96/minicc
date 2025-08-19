#!/bin/bash

../build/minicc ../main.c > ../main.s
riscv64-linux-gnu-gcc -static -o ../main ../main.s
qemu-riscv64 ../main
echo "Exit code: $?"