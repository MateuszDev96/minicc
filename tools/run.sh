./build.sh
../build/minicc -c ../main.cwe
riscv64-linux-gnu-gcc -static -o ../main ../main.s
qemu-riscv64 ../main

echo -e "\n Exit code: $?"
