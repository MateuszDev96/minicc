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
            if (node->var->offset >= -2048 && node->var->offset <= 2047) {
                fprintf(out, "  addi a0, s0, %d\n", node->var->offset);
            } else {
                fprintf(out, "  li t0, %d\n", node->var->offset);
                fprintf(out, "  add a0, s0, t0\n");
            }
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
    if (ty->kind == TY_ARRAY) return;
    fprintf(out, "  ld a0, 0(a0)\n");
}

static void store(FILE *out) {
    pop("a1", out);
    fprintf(out, "  sd a0, 0(a1)\n");
}

static void gen_newline(FILE *out) {
    fprintf(out,
        "  la a1, .L.nl\n"
        "  li a2, 1\n"
        "  li a0, 1\n"
        "  li a7, 64\n"
        "  ecall\n");
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
            fprintf(out, "  .section .rodata\n");
            fprintf(out, ".L.str.%d:\n", label);
            fprintf(out, "  .asciz \"%s\"\n", node->str);
            fprintf(out, "  .section .text\n");
            fprintf(out, "  la a0, .L.str.%d\n", label);
            return;
        }
        case ND_FUNCALL: {
            Node *arg = node->args;

            if (strcmp(node->funcname, "print") == 0) {
                for (Node *a = node->args; a; a = a->next) {
                    if (a->kind == ND_STRING) {
                        gen_expr(a, out);
                        int len = (int)strlen(a->str);
                        fprintf(out,
                            "  mv a1, a0\n"
                            "  li a2, %d\n"
                            "  li a0, 1\n"
                            "  li a7, 64\n"
                            "  ecall\n", len);
                    } else {
                        gen_expr(a, out);
                        fprintf(out, "  call print_num\n");
                    }
                }
                gen_newline(out);
                return;
            }

            int nargs = 0;
            for (Node *a = node->args; a; a = a->next) {
                gen_expr(a, out);
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
            fprintf(out, "  sub t0, a0, a1\n  seqz a0, t0\n");
            return;
        case ND_NE:
            fprintf(out, "  sub t0, a0, a1\n  snez a0, t0\n");
            return;
        case ND_LT:
            fprintf(out, "  slt a0, a0, a1\n");
            return;
        case ND_LE:
            fprintf(out, "  slt a0, a1, a0\n  xori a0, a0, 1\n");
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
            if (node->els) gen_stmt(node->els, out);
            fprintf(out, ".L.end.%d:\n", c);
            return;
        }
        case ND_FOR: {
            int c = count();
            if (node->init) gen_stmt(node->init, out);
            fprintf(out, ".L.begin.%d:\n", c);
            if (node->cond) {
                gen_expr(node->cond, out);
                fprintf(out, "  beqz a0, .L.end.%d\n", c);
            }
            gen_stmt(node->then, out);
            if (node->inc) gen_expr(node->inc, out);
            fprintf(out, "  j .L.begin.%d\n", c);
            fprintf(out, ".L.end.%d:\n", c);
            return;
        }
        case ND_BLOCK:
            for (Node *n = node->body; n; n = n->next) gen_stmt(n, out);
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
            offset += align_to(var->ty->size, 8);  // wyrównanie do 8 bajtów
            var->offset = -offset;
        }
        fn->stack_size = align_to(offset, 16);
    }
}

static void gen_print_num_function(FILE *out) {
    fprintf(out,
        "  .globl print_num\n"
        "print_num:\n"
        "  addi sp, sp, -112\n"
        "  sd s0, 0(sp)\n"
        "  sd s1, 8(sp)\n"
        "  sd s2, 16(sp)\n"
        "  sd s3, 24(sp)\n"
        "  sd ra, 32(sp)\n"
        "  mv s0, a0\n"
        "  li s1, 10\n"
        "  li s2, 0\n"
        "  mv s3, sp\n"
        "  addi s3, s3, 111\n"
        "1:\n"
        "  beqz s0, 2f\n"
        "  rem t1, s0, s1\n"
        "  div s0, s0, s1\n"
        "  addi t1, t1, 48\n"
        "  addi s3, s3, -1\n"
        "  sb t1, 0(s3)\n"
        "  addi s2, s2, 1\n"
        "  j 1b\n"
        "2:\n"
        "  beqz s2, 3f\n"
        "  j 4f\n"
        "3:\n"
        "  addi s3, s3, -1\n"
        "  li t1, 48\n"
        "  sb t1, 0(s3)\n"
        "  li s2, 1\n"
        "4:\n"
        "  li a0, 1\n"
        "  mv a1, s3\n"
        "  mv a2, s2\n"
        "  li a7, 64\n"
        "  ecall\n"
        "  la a1, .L.space\n"
        "  li a2, 1\n"
        "  li a0, 1\n"
        "  li a7, 64\n"
        "  ecall\n"
        "  li a0, 0\n"
        "  ld s0, 0(sp)\n"
        "  ld s1, 8(sp)\n"
        "  ld s2, 16(sp)\n"
        "  ld s3, 24(sp)\n"
        "  ld ra, 32(sp)\n"
        "  addi sp, sp, 112\n"
        "  ret\n"
    );
}

void codegen(Function *prog, FILE *out) {
    assign_lvar_offsets(prog);

    fprintf(out, "  .section .rodata\n");
    fprintf(out, ".L.nl:\n  .asciz \"\\n\"\n");
    fprintf(out, ".L.space:\n  .asciz \" \"\n");
    fprintf(out, "  .section .text\n");

    gen_print_num_function(out);

    for (Function *fn = prog; fn; fn = fn->next) {
        fprintf(out, "  .globl %s\n", fn->name);
        fprintf(out, "%s:\n", fn->name);
        current_fn = fn;

        fprintf(out,
            "  addi sp, sp, -16\n"
            "  sd ra, 8(sp)\n"
            "  sd s0, 0(sp)\n"
            "  mv s0, sp\n");

        fprintf(out, "  li t0, %d\n", fn->stack_size);
        fprintf(out, "  sub sp, sp, t0\n");

        int i = 0;
        for (Obj *var = fn->params; var; var = var->next) {
            fprintf(out, "  sd %s, %d(s0)\n", argreg[i++], var->offset);
        }

        gen_stmt(fn->body, out);
        assert(depth == 0);

        fprintf(out,
            ".L.return.%s:\n"
            "  mv sp, s0\n"
            "  ld s0, 0(sp)\n"
            "  ld ra, 8(sp)\n"
            "  addi sp, sp, 16\n"
            "  ret\n", fn->name);
    }
}
