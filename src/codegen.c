#include <stdio.h>
#include "minicc.h"

static int depth;
static char *argreg[] = { "x0", "x1", "x2", "x3", "x4", "x5" };
static Function *current_fn;

static void gen_expr(Node *node, FILE *out);

static int count(void) {
  static int i = 1;
  return i++;
}

static void push(FILE *out) {
  fprintf(out, "  sub sp, sp, #8\n");
  fprintf(out, "  str x0, [sp]\n");
  depth++;
}

static void pop(char *reg, FILE *out) {
  fprintf(out, "  ldr %s, [sp]\n", reg);
  fprintf(out, "  add sp, sp, #8\n");
  depth--;
}

static void gen_mov_imm64(unsigned long long val, FILE *out) {
  int first = 1;
  for (int i = 0; i < 4; i++) {
    unsigned short part = (val >> (16 * i)) & 0xFFFF;
    if (first) {
      fprintf(out, "  movz x0, #%u", part);
      if (i > 0) fprintf(out, ", lsl #%d", i * 16);
      fprintf(out, "\n");
      first = 0;
    } else {
      if (part != 0) {
        fprintf(out, "  movk x0, #%u, lsl #%d\n", part, i * 16);
      }
    }
  }
}


static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

static void gen_addr(Node *node, FILE *out) {
  switch (node->kind) {
    case ND_VAR: {
      fprintf(out, "  add x0, x29, #%d\n", node->var->offset);
      return;
    }

    case ND_DEREF: {
      gen_expr(node->lhs, out);
      return;
    }

    default:
      break;
  }

  error_tok(node->tok, "not an lvalue");
}

static void load(Type *ty, FILE *out) {
  if (ty->kind == TY_ARRAY) {
    return;
  }

  fprintf(out, "  ldr x0, [x0]\n");
}

static void store(FILE *out) {
  pop("x1", out);
  fprintf(out, "  str x0, [x1]\n");
}

static void gen_expr(Node *node, FILE *out) {
  switch (node->kind) {
    case ND_NUM: {
      gen_mov_imm64((unsigned long long)node->val, out);
      return;
    }

    case ND_NEG: {
      gen_expr(node->lhs, out);
      fprintf(out, "  neg x0, x0\n");
      return;
    }

    case ND_VAR: {
      gen_addr(node, out);
      load(node->ty, out);
      return;
    }

    case ND_DEREF: {
      gen_expr(node->lhs, out);
      load(node->ty, out);
      return;
    }

    case ND_ADDR: {
      gen_addr(node->lhs, out);
      return;
    }

    case ND_ASSIGN: {
      gen_addr(node->lhs, out);
      push(out);
      gen_expr(node->rhs, out);
      store(out);
      return;
    }

    case ND_FUNCALL: {
      if (strcmp(node->funcname, "print") == 0) {
        gen_expr(node->args, out);
        fprintf(out, "  mov x8, #64\n");   // syscall write
        fprintf(out, "  svc #0\n");
        return;
      }

      int nargs = 0;
      for (Node *arg = node->args; arg; arg = arg->next) {
        gen_expr(arg, out);
        push(out);
        nargs++;
      }

      for (int i = nargs - 1; i >= 0; i--) {
        pop(argreg[i], out);
      }

      fprintf(out, "  bl %s\n", node->funcname);
      return;
    }
    default:
      break;
  }

  gen_expr(node->rhs, out);
  push(out);
  gen_expr(node->lhs, out);
  pop("x1", out);

  switch (node->kind) {
    case ND_ADD: {
      fprintf(out, "  add x0, x0, x1\n");
      return;
    }

    case ND_SUB: {
      fprintf(out, "  sub x0, x0, x1\n");
      return;
    }

    case ND_MUL: {
      fprintf(out, "  mul x0, x0, x1\n");
      return;
    }

    case ND_DIV: {
      fprintf(out, "  sdiv x0, x0, x1\n");
      return;
    }

    case ND_EQ:
    case ND_NE: {
      fprintf(out, "  eor x0, x0, x1\n");

      if (node->kind == ND_EQ) {
        fprintf(out, "  cset x0, eq\n");
      } else {
        fprintf(out, "  cset x0, ne\n");
      }

      return;
    }

    case ND_LT: {
      fprintf(out, "  cset x0, lt\n");
      fprintf(out, "  cmp x0, x1\n"); // cmp before cset normally, but simplified here
      return;
    }

    case ND_LE: {
      fprintf(out, "  cset x0, le\n");
      fprintf(out, "  cmp x0, x1\n");
      return;
    }

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
      fprintf(out, "  cbz x0, .L.else.%d\n", c);
      gen_stmt(node->then, out);
      fprintf(out, "  b .L.end.%d\n", c);
      fprintf(out, ".L.else.%d:\n", c);

      if (node->els) {
        gen_stmt(node->els, out);
      }

      fprintf(out, ".L.end.%d:\n", c);
      return;
    }

    case ND_FOR: {
      int c = count();

      if (node->init) {
        gen_stmt(node->init, out);
      }

      fprintf(out, ".L.begin.%d:\n", c);

      if (node->cond) {
        gen_expr(node->cond, out);
        fprintf(out, "  cbz x0, .L.end.%d\n", c);
      }

      gen_stmt(node->then, out);

      if (node->inc) {
        gen_expr(node->inc, out);
      }

      fprintf(out, "  b .L.begin.%d\n", c);
      fprintf(out, ".L.end.%d:\n", c);
      return;
    }

    case ND_BLOCK: {
      for (Node *n = node->body; n; n = n->next) {
        gen_stmt(n, out);
      }

      return;
    }

    case ND_RETURN: {
      gen_expr(node->lhs, out);
      fprintf(out, "  b .L.return.%s\n", current_fn->name);
      return;
    }

    case ND_EXPR_STMT: {
      gen_expr(node->lhs, out);
      return;
    }

    default: {
      break;
    }
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

    fprintf(out, "  sub sp, sp, #16\n");
    fprintf(out, "  str x30, [sp, #8]\n");   // save lr
    fprintf(out, "  str x29, [sp]\n");       // save fp
    fprintf(out, "  mov x29, sp\n");         // set fp = sp
    fprintf(out, "  sub sp, sp, #%d\n", fn->stack_size);

    int i = 0;
    for (Obj *var = fn->params; var; var = var->next) {
      fprintf(out, "  str %s, [x29, #%d]\n", argreg[i++], var->offset);
    }

    gen_stmt(fn->body, out);
    assert(depth == 0);

    fprintf(out, ".L.return.%s:\n", fn->name);
    fprintf(out, "  mov sp, x29\n");
    fprintf(out, "  ldr x29, [sp]\n");       // restore fp
    fprintf(out, "  ldr x30, [sp, #8]\n");   // restore lr
    fprintf(out, "  add sp, sp, #16\n");
    fprintf(out, "  ret\n");
  }
}
