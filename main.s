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
  mv s0, a0
  li s1, 10
  li s2, 0
  mv s3, sp
  addi s3, s3, 111
1:
  beqz s0, 2f
  rem t1, s0, s1
  div s0, s0, s1
  addi t1, t1, 48
  addi s3, s3, -1
  sb t1, 0(s3)
  addi s2, s2, 1
  j 1b
2:
  beqz s2, 3f
  j 4f
3:
  addi s3, s3, -1
  li t1, 48
  sb t1, 0(s3)
  li s2, 1
4:
  li a0, 1
  mv a1, s3
  mv a2, s2
  li a7, 64
  ecall
  la a1, .L.space
  li a2, 1
  li a0, 1
  li a7, 64
  ecall
  li a0, 0
  ld s0, 0(sp)
  ld s1, 8(sp)
  ld s2, 16(sp)
  ld s3, 24(sp)
  ld ra, 32(sp)
  addi sp, sp, 112
  ret
  .globl main
main:
  addi sp, sp, -16
  sd ra, 8(sp)
  sd s0, 0(sp)
  mv s0, sp
  li t0, 16
  sub sp, sp, t0
  addi a0, s0, -16
  addi sp, sp, -8
  sd a0, 0(sp)
  li a0, 1
  ld a1, 0(sp)
  addi sp, sp, 8
  sd a0, 0(a1)
  addi a0, s0, -8
  addi sp, sp, -8
  sd a0, 0(sp)
  li a0, 192
  ld a1, 0(sp)
  addi sp, sp, 8
  sd a0, 0(a1)
  addi a0, s0, -8
  ld a0, 0(a0)
  addi sp, sp, -8
  sd a0, 0(sp)
  addi a0, s0, -16
  ld a0, 0(a0)
  ld a1, 0(sp)
  addi sp, sp, 8
  add a0, a0, a1
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
