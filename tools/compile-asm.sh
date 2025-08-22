#!/bin/bash
aarch64-linux-gnu-gcc -static -o ../main ../main.s
qemu-aarch64 ../main
echo "Exit code: $?"
