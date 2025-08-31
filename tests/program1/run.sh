#!/bin/bash
./build/minicc ./tests/program1/program1.cwe ./tests/program1/program1.s
riscv64-linux-gnu-gcc -static -o ./tests/program1/program1 ./tests/program1/program1.s
qemu-riscv64 ./tests/program1/program1

echo -e "\n Exit code: $?"
