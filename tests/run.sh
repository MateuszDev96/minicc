#!/bin/bash
./build/minicc ./tests/program1/program1.cwe ./tests/program1/program1.s
riscv64-linux-gnu-gcc -static -o ./tests/program1/program1.exec ./tests/program1/program1.s
qemu-riscv64 ./tests/program1/program1.exec

if [ $? = 0 ]; then
  echo "OK program1"
else
  echo "FAIL program1"
  exit 1
fi

./build/minicc ./tests/program2/program2.cwe ./tests/program2/program2.s
riscv64-linux-gnu-gcc -static -o ./tests/program2/program2.exec ./tests/program2/program2.s
qemu-riscv64 ./tests/program2/program2.exec

if [ $? = 42 ]; then
  echo "OK program2"
else
  echo "FAIL program2"
  exit 1
fi

./build/minicc ./tests/program3/program3.cwe ./tests/program3/program3.s
riscv64-linux-gnu-gcc -static -o ./tests/program3/program3.exec ./tests/program3/program3.s
qemu-riscv64 ./tests/program3/program3.exec

if [ $? = 114 ]; then
  echo "OK program3"
else
  echo "FAIL program3"
  exit 1
fi

