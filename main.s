  .section .rodata
.L.nl:
  .asciz "\n"
.L.space:
  .asciz " "
  .section .text
  .globl print_num
print_num:
  addi sp, sp, -112
  sd s0, 0(sp)
  sd s1, 8(sp)
  sd s2, 16(sp)
  sd s3, 24(sp)
  sd ra, 32(sp)
  mv s0, a0           # n
  li s1, 10           # base
  li s2, 0            # length
  mv s3, sp           # buf pointer
  addi s3, s3, 111    # buf end
1:
  beqz s0, 2f         # if n == 0 goto print
  rem t1, s0, s1      # digit = n % 10
  div s0, s0, s1      # n = n / 10
  addi t1, t1, 48     # convert digit to ascii
  addi s3, s3, -1     # move back buf pointer
  sb t1, 0(s3)        # store digit
  addi s2, s2, 1      # length++
  j 1b
2:
  beqz s2, 3f         # if length==0 (means original n==0)
  j 4f
3:
  addi s3, s3, -1
  li t1, 48           # ascii '0'
  sb t1, 0(s3)
  li s2, 1
4:
  li a0, 1            # stdout fd
  mv a1, s3           # buf pointer
  mv a2, s2           # length
  li a7, 64           # syscall write
  ecall
  la a1, .L.space
  li a2, 1
  li a0, 1
  li a7, 64
  ecall
  li a0, 0            # return 0
  ld s0, 0(sp)
  ld s1, 8(sp)
  ld s2, 16(sp)
  ld s3, 24(sp)
  ld ra, 32(sp)
  addi sp, sp, 112
  ret
  .globl hehe
hehe:
  addi sp, sp, -16
  sd ra, 8(sp)
  sd s0, 0(sp)
  mv s0, sp
  li t0, 0
  sub sp, sp, t0
  li a0, 1
  j .L.return.hehe
.L.return.hehe:
  mv sp, s0
  ld s0, 0(sp)
  ld ra, 8(sp)
  addi sp, sp, 16
  ret
  .globl main
main:
  addi sp, sp, -16
  sd ra, 8(sp)
  sd s0, 0(sp)
  mv s0, sp
  li t0, 8000
  sub sp, sp, t0
  li a0, 8
  addi sp, sp, -8
  sd a0, 0(sp)
  li a0, 1
  ld a1, 0(sp)
  addi sp, sp, 8
  mul a0, a0, a1
  addi sp, sp, -8
  sd a0, 0(sp)
  li t0, -8000
  add a0, s0, t0
  ld a1, 0(sp)
  addi sp, sp, 8
  add a0, a0, a1
  addi sp, sp, -8
  sd a0, 0(sp)
  li a0, 9
  ld a1, 0(sp)
  addi sp, sp, 8
  sd a0, 0(a1)
  li a0, 8
  addi sp, sp, -8
  sd a0, 0(sp)
  li a0, 2
  ld a1, 0(sp)
  addi sp, sp, 8
  mul a0, a0, a1
  addi sp, sp, -8
  sd a0, 0(sp)
  li t0, -8000
  add a0, s0, t0
  ld a1, 0(sp)
  addi sp, sp, 8
  add a0, a0, a1
  addi sp, sp, -8
  sd a0, 0(sp)
  li a0, 9
  ld a1, 0(sp)
  addi sp, sp, 8
  sd a0, 0(a1)
  li a0, 8
  addi sp, sp, -8
  sd a0, 0(sp)
  li a0, 3
  ld a1, 0(sp)
  addi sp, sp, 8
  mul a0, a0, a1
  addi sp, sp, -8
  sd a0, 0(sp)
  li t0, -8000
  add a0, s0, t0
  ld a1, 0(sp)
  addi sp, sp, 8
  add a0, a0, a1
  addi sp, sp, -8
  sd a0, 0(sp)
  li a0, 7
  ld a1, 0(sp)
  addi sp, sp, 8
  sd a0, 0(a1)
  li a0, 8
  call print_num
  la a1, .L.nl
  li a2, 1
  li a0, 1
  li a7, 64
  ecall
  li a0, 0
  j .L.return.main
.L.return.main:
  mv sp, s0
  ld s0, 0(sp)
  ld ra, 8(sp)
  addi sp, sp, 16
  ret
