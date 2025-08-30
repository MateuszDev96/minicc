  .globl main
main:
  addi sp, sp, -16
  sd ra, 8(sp)
  sd s0, 0(sp)
  mv s0, sp
  addi sp, sp, -0
  li a0, 123
  j .L.return.main
.L.return.main:
  mv sp, s0
  ld s0, 0(sp)
  ld ra, 8(sp)
  addi sp, sp, 16
  ret
