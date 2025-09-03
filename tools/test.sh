#!/bin/bash

assert 5 'int main() { int x = 3; int y = 5; return *(&x + 1); }' # 52
assert 3 'int main() { int x = 3; int y = 5; return *(&y - 1); }' # 53
assert 5 'int main() { int x = 3; int y = 5; return *(&x - (-1)); }' # 54
assert 5 'int main() { int x = 3; int *y = &x; *y = 5; return x; }' # 55
assert 7 'int main() { int x = 3; int y = 5; *(&x + 1) = 7; return y; }' # 56
assert 7 'int main() { int x = 3; int y = 5; *(&y - 2 + 1) = 7; return x; }' # 57
assert 5 'int main() { int x = 3; return (&x + 2) - &x + 3; }' # 58
assert 8 'int main() { int x, y; x = 3; y = 5; return x + y; }' # 59
assert 8 'int main() { int x = 3, y = 5; return x + y; }' # 60

assert 3 'int main() { return ret3(); }'
assert 5 'int main() { return ret5(); }'
assert 8 'int main() { return add(3, 5); }'
assert 2 'int main() { return sub(5, 3); }'
assert 21 'int main() { return add6(1, 2, 3, 4, 5, 6); }'
assert 66 'int main() { return add6(1, 2, add6(3, 4, 5, 6, 7, 8), 9, 10, 11); }'
assert 136 'int main() { return add6(1, 2, add6(3, add6(4, 5, 6, 7, 8, 9), 10, 11, 12, 13), 14, 15, 16); }'

assert 32 'int main() { return ret32(); } int ret32() { return 32; }'

assert 7 'int main() { return add2(3, 4); } int add2(int x, int y) { return x + y; }'
assert 1 'int main() { return sub2(4, 3); } int sub2(int x, int y) { return x - y; }'
assert 55 'int main() { return fib(9); } int fib(int x) { if (x <= 1) return 1; return fib(x - 1) + fib(x - 2); }'

assert 3 'int main() { int x[2]; int *y = &x; *y = 3; return *x; }' # 61
assert 3 'int main() { int x[3]; *x = 3; *(x + 1) = 4; *(x + 2) = 5; return *x; }' # 62
assert 4 'int main() { int x[3]; *x = 3; *(x + 1) = 4; *(x + 2) = 5; return *(x + 1); }' # 63
assert 5 'int main() { int x[3]; *x = 3; *(x + 1) = 4; *(x + 2) = 5; return *(x + 2); }' # 64

assert 0 'int main() { int x[2][3]; int *y = x; *y = 0; return **x; }' # 65
assert 1 'int main() { int x[2][3]; int *y = x; *(y + 1) = 1; return *(*x + 1); }' # 66
assert 2 'int main() { int x[2][3]; int *y = x; *(y + 2) = 2; return *(*x + 2); }' # 67
assert 3 'int main() { int x[2][3]; int *y = x; *(y + 3) = 3; return **(x + 1); }' # 68
assert 4 'int main() { int x[2][3]; int *y = x; *(y + 4) = 4; return *(*(x + 1) + 1); }' # 69
assert 5 'int main() { int x[2][3]; int *y = x; *(y + 5) = 5; return *(*(x + 1) + 2); }' # 70

assert 0 'int main() { int x[2][3]; int *y = x; y[0] = 0; return x[0][0]; }' # 71
assert 1 'int main() { int x[2][3]; int *y = x; y[1] = 1; return x[0][1]; }' # 72
assert 2 'int main() { int x[2][3]; int *y = x; y[2] = 2; return x[0][2]; }' # 73
assert 3 'int main() { int x[2][3]; int *y = x; y[3] = 3; return x[1][0]; }' # 74
assert 4 'int main() { int x[2][3]; int *y = x; y[4] = 4; return x[1][1]; }'
assert 5 'int main() { int x[2][3]; int *y = x; y[5] = 5; return x[1][2]; }'

assert 8 'int main() { int x; return sizeof(x); }'
assert 8 'int main() { int x; return sizeof x; }'
assert 8 'int main() { int *x; return sizeof(x); }'
assert 32 'int main() { int x[4]; return sizeof(x); }'
assert 96 'int main() { int x[3][4]; return sizeof(x); }'
assert 32 'int main() { int x[3][4]; return sizeof(*x); }'
assert 8 'int main() { int x[3][4]; return sizeof(**x); }'
assert 9 'int main() { int x[3][4]; return sizeof(**x) + 1; }'
assert 9 'int main() { int x[3][4]; return sizeof **x + 1; }'
assert 8 'int main() { int x[3][4]; return sizeof(**x + 1); }'
assert 8 'int main() { int x = 1; return sizeof(x = 2); }'
assert 1 'int main() { int x = 1; sizeof(x = 2); return x; }'

echo OK
