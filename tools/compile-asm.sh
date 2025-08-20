#!/bin/bash
riscv64-linux-gnu-gcc -static -o ../main ../main.s
qemu-riscv64 ../main
echo "Exit code: $?"
