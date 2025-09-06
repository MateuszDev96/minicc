#!/bin/bash
./tools/build.sh
./build/minicc ./main.cwe ./main.s
riscv64-linux-gnu-gcc -static -o ./main.exec ./main.s
qemu-riscv64 ./main.exec

echo -e "\n Exit code: $?"
