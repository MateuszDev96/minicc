#include <stdio.h>
#include "minicc.h"

static int depth;
static char *argreg[] = { "a0", "a1", "a2", "a3", "a4", "a5" };
static Function *current_fn;

static void gen_expr(Node *node, FILE *out);

static int count(void) {
  static int i = 1;
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

static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

static void gen_addr(Node *node, FILE *out) {
  switch (node->kind) {
    case ND_VAR: {
      fprintf(out, "  addi a0, fp, %d\n", node->var->offset);
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

  fprintf(out, "  ld a0, 0(a0)\n");
}

static void store(FILE *out) {
  pop("a1", out);
  fprintf(out, "  sd a0, 0(a1)\n");
}

static void gen_expr(Node *node, FILE *out) {
  switch (node->kind) {
    case ND_NUM: {
      fprintf(out, "  li a0, %d\n", node->val);
      return;
    }

    case ND_NEG: {
      gen_expr(node->lhs, out);
      fprintf(out, "  neg a0, a0\n");
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
        fprintf(out, "  li a7, 1\n");
        fprintf(out, "  ecall\n");
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
    case ND_ADD: {
      fprintf(out, "  add a0, a0, a1\n");
      return;
    }

    case ND_SUB: {
      fprintf(out, "  sub a0, a0, a1\n");
      return;
    }

    case ND_MUL: {
      fprintf(out, "  mul a0, a0, a1\n");
      return;
    }

    case ND_DIV: {
      fprintf(out, "  div a0, a0, a1\n");
      return;
    }

    case ND_EQ:
    case ND_NE: {
      fprintf(out, "  xor a0, a0, a1\n");

      if (node->kind == ND_EQ) {
        fprintf(out, "  seqz a0, a0\n");
      } else {
        fprintf(out, "  snez a0, a0\n");
      }

      return;
    }

    case ND_LT: {
      fprintf(out, "  slt a0, a0, a1\n");
      return;
    }

    case ND_LE: {
      fprintf(out, "  slt a0, a1, a0\n");
      fprintf(out, "  xori a0, a0, 1\n");
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
      fprintf(out, "  beqz a0, .L.else.%d\n", c);
      gen_stmt(node->then, out);
      fprintf(out, "  j .L.end.%d\n", c);
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
        fprintf(out, "  beqz a0, .L.end.%d\n", c);
      }

      gen_stmt(node->then, out);

      if (node->inc) {
        gen_expr(node->inc, out);
      }

      fprintf(out, "  j .L.begin.%d\n", c);
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
      fprintf(out, "  j .L.return.%s\n", current_fn->name);
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

    fprintf(out, "  addi sp, sp, -16\n");
    fprintf(out, "  sd ra, 8(sp)\n");
    fprintf(out, "  sd fp, 0(sp)\n");
    fprintf(out, "  mv fp, sp\n");
    fprintf(out, "  addi sp, sp, %d\n", -fn->stack_size);

    int i = 0;
    for (Obj *var = fn->params; var; var = var->next) {
      fprintf(out, "  sd %s, %d(fp)\n", argreg[i++], var->offset);
    }

    gen_stmt(fn->body, out);
    assert(depth == 0);

    fprintf(out, ".L.return.%s:\n", fn->name);
    fprintf(out, "  mv sp, fp\n");
    fprintf(out, "  ld fp, 0(sp)\n");
    fprintf(out, "  ld ra, 8(sp)\n");
    fprintf(out, "  addi sp, sp, 16\n");
    fprintf(out, "  ret\n");
  }
}

