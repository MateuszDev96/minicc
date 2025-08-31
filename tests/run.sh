# ./build/minicc -c ./tests/program1.cwe
./build/minicc "int main() { return 111; }"
riscv64-linux-gnu-gcc -static -o ./main ./main.s
qemu-riscv64 ./main

echo -e "\n Exit code: $?"
