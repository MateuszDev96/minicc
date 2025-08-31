
# expected=0
# input="int main() {return 0;}"

# ./build/minicc "$input" > ./build/tmp.s || exit
./build.sh
./build/minicc -c ./tests/program1.cwe
riscv64-linux-gnu-gcc -static -o ./tests/program1 ./tests/program1.s
# riscv64-linux-gnu-gcc -static -o ./build ./main.s
# qemu-riscv64-static ./build/tmp

# ../build/minicc -c ../main.cwe

# actual="$?"

# if [ "$actual" = "$expected" ]; then
#   echo "okkk"
# else
#   echo "$input => $expected expected, but got $actual"
#   exit 1
# fi
