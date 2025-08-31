#!/bin/bash
./tools/build.sh
./build/minicc ./main.cwe ./main.s
riscv64-linux-gnu-gcc -static -o ./main ./main.s
qemu-riscv64 ./main

echo -e "\n Exit code: $?"
