#!/bin/bash
./build/minicc ../tests/program1/program1.cwe ../tests/program1/program1.s
riscv64-linux-gnu-gcc -static -o ../tests/program1/program1.exec ../tests/program1/program1.s
qemu-riscv64 ../tests/program1/program1.exec

if [ $? = 0 ]; then
  echo "OK $0"
else
  echo "FAIL $0"
  exit 1
fi
