./build.sh
../build/minicc -c ../main.c
aarch64-linux-gnu-gcc -static -o ../main ../main.s
qemu-aarch64 ../main

echo -e "\n Exit code: $?"
