#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "minicc.h"

static int depth;
static char *argreg[] = { "a0", "a1", "a2", "a3", "a4", "a5" };
static Function *current_fn;

static void gen_expr(Node *node, FILE *out);

static unsigned long long count(void) {
  static unsigned long long i = 1;
  return i++;
}

static void push(FILE *out) {
  fprintf(out, "  addi sp, sp, -8\n");
  fprintf(out, "  sd a0, 0(sp)\n");
  depth++;
}

static void pop(char *reg, FILE *out) {
  fprintf(out, "  ld %s, 0(sp)\n", reg);
  fprintf(out, "  addi sp, sp, 8\n");
  depth--;
}

static void gen_mov_imm64(unsigned long long val, FILE *out) {
  fprintf(out, "  li a0, %llu\n", val);
}

static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

static void gen_addr(Node *node, FILE *out) {
  switch (node->kind) {
    case ND_VAR:
      fprintf(out, "  addi a0, s0, %d\n", node->var->offset);
      return;
    case ND_DEREF:
      gen_expr(node->lhs, out);
      return;
    default:
      break;
  }
  error_tok(node->tok, "not an lvalue");
}

static void load(Type *ty, FILE *out) {
  if (ty->kind == TY_ARRAY)
    return;
  fprintf(out, "  ld a0, 0(a0)\n");
}

static void store(FILE *out) {
  pop("a1", out);
  fprintf(out, "  sd a0, 0(a1)\n");
}

static void gen_expr(Node *node, FILE *out) {
  switch (node->kind) {
    case ND_NUM:
      gen_mov_imm64((unsigned long long)node->val, out);
      return;

    case ND_NEG:
      gen_expr(node->lhs, out);
      fprintf(out, "  neg a0, a0\n");
      return;

    case ND_VAR:
      gen_addr(node, out);
      load(node->ty, out);
      return;

    case ND_DEREF:
      gen_expr(node->lhs, out);
      load(node->ty, out);
      return;

    case ND_ADDR:
      gen_addr(node->lhs, out);
      return;

    case ND_ASSIGN:
      gen_addr(node->lhs, out);
      push(out);
      gen_expr(node->rhs, out);
      store(out);
      return;

    case ND_STRING: {
      int label = count();
      fprintf(out, ".L.str.%d:\n", label);
      fprintf(out, "  .asciz \"%s\"\n", node->str);
      fprintf(out, "  la a0, .L.str.%d\n", label);
      return;
    }

    case ND_FUNCALL: {
      Node *arg = node->args;

      if (strcmp(node->funcname, "print") == 0) {
        if (arg->kind == ND_STRING) {
          gen_expr(arg, out);
          int len = strlen(arg->str);
          fprintf(out,
            "  mv a1, a0\n"
            "  li a2, %d\n"
            "  li a0, 1\n"
            "  li a7, 64\n"
            "  ecall\n", len);
          return;
        } else if (arg->kind == ND_NUM) {
          // Tu możesz zamiast syscall wywołać print_num (musisz ją napisać)
          // lub generować call print (wymaga definicji print).
          gen_expr(arg, out);       // załaduj wartość liczby w a0
          fprintf(out, "  call print_num\n"); // zakładamy istnienie print_num
          return;
        } else {
          // domyślnie generujemy call print
          // generuj argumenty normalnie:
          int nargs = 0;
          for (Node *arg2 = node->args; arg2; arg2 = arg2->next) {
            gen_expr(arg2, out);
            push(out);
            nargs++;
          }
          for (int i = nargs - 1; i >= 0; i--) {
            pop(argreg[i], out);
          }
          fprintf(out, "  call %s\n", node->funcname);
          return;
        }
      }

      // Inne funkcje
      int nargs = 0;
      for (Node *arg2 = node->args; arg2; arg2 = arg2->next) {
        gen_expr(arg2, out);
        push(out);
        nargs++;
      }
      for (int i = nargs - 1; i >= 0; i--) {
        pop(argreg[i], out);
      }
      fprintf(out, "  call %s\n", node->funcname);
      return;
    }

    default:
      break;
  }

  gen_expr(node->rhs, out);
  push(out);
  gen_expr(node->lhs, out);
  pop("a1", out);

  switch (node->kind) {
    case ND_ADD:
      fprintf(out, "  add a0, a0, a1\n");
      return;

    case ND_SUB:
      fprintf(out, "  sub a0, a0, a1\n");
      return;

    case ND_MUL:
      fprintf(out, "  mul a0, a0, a1\n");
      return;

    case ND_DIV:
      fprintf(out, "  div a0, a0, a1\n");
      return;

    case ND_EQ:
      fprintf(out, "  sub t0, a0, a1\n");
      fprintf(out, "  seqz a0, t0\n");
      return;

    case ND_NE:
      fprintf(out, "  sub t0, a0, a1\n");
      fprintf(out, "  snez a0, t0\n");
      return;

    case ND_LT:
      fprintf(out, "  slt a0, a0, a1\n");
      return;

    case ND_LE:
      fprintf(out, "  slt a0, a1, a0\n");
      fprintf(out, "  xori a0, a0, 1\n");
      return;

    default:
      break;
  }

  error_tok(node->tok, "invalid expression");
}

static void gen_stmt(Node *node, FILE *out) {
  switch (node->kind) {
    case ND_IF: {
      int c = count();
      gen_expr(node->cond, out);
      fprintf(out, "  beqz a0, .L.else.%d\n", c);
      gen_stmt(node->then, out);
      fprintf(out, "  j .L.end.%d\n", c);
      fprintf(out, ".L.else.%d:\n", c);
      if (node->els)
        gen_stmt(node->els, out);
      fprintf(out, ".L.end.%d:\n", c);
      return;
    }

    case ND_FOR: {
      int c = count();
      if (node->init)
        gen_stmt(node->init, out);
      fprintf(out, ".L.begin.%d:\n", c);
      if (node->cond) {
        gen_expr(node->cond, out);
        fprintf(out, "  beqz a0, .L.end.%d\n", c);
      }
      gen_stmt(node->then, out);
      if (node->inc)
        gen_expr(node->inc, out);
      fprintf(out, "  j .L.begin.%d\n", c);
      fprintf(out, ".L.end.%d:\n", c);
      return;
    }

    case ND_BLOCK:
      for (Node *n = node->body; n; n = n->next)
        gen_stmt(n, out);
      return;

    case ND_RETURN:
      gen_expr(node->lhs, out);
      fprintf(out, "  j .L.return.%s\n", current_fn->name);
      return;

    case ND_EXPR_STMT:
      gen_expr(node->lhs, out);
      return;

    default:
      break;
  }

  error_tok(node->tok, "invalid statement");
}

static void assign_lvar_offsets(Function *prog) {
  for (Function *fn = prog; fn; fn = fn->next) {
    int offset = 0;
    for (Obj *var = fn->locals; var; var = var->next) {
      offset += var->ty->size;
      var->offset = -offset;
    }
    fn->stack_size = align_to(offset, 16);
  }
}

void codegen(Function *prog, FILE *out) {
  assign_lvar_offsets(prog);

  for (Function *fn = prog; fn; fn = fn->next) {
    fprintf(out, "  .globl %s\n", fn->name);
    fprintf(out, "%s:\n", fn->name);
    current_fn = fn;

    fprintf(out, "  addi sp, sp, -16\n");
    fprintf(out, "  sd ra, 8(sp)\n");
    fprintf(out, "  sd s0, 0(sp)\n");
    fprintf(out, "  mv s0, sp\n");
    fprintf(out, "  addi sp, sp, -%d\n", fn->stack_size);

    int i = 0;
    for (Obj *var = fn->params; var; var = var->next) {
      fprintf(out, "  sd %s, %d(s0)\n", argreg[i++], var->offset);
    }

    gen_stmt(fn->body, out);
    assert(depth == 0);

    fprintf(out, ".L.return.%s:\n", fn->name);
    fprintf(out, "  mv sp, s0\n");
    fprintf(out, "  ld s0, 0(sp)\n");
    fprintf(out, "  ld ra, 8(sp)\n");
    fprintf(out, "  addi sp, sp, 16\n");
    fprintf(out, "  ret\n");
  }
}
