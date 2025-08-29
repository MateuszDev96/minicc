./build.sh
../build/minicc -c ../main.c
riscv64-linux-gnu-gcc -static -o ../main ../main.s
qemu-riscv64 ../main

echo -e "\n Exit code: $?"
