#include "minicc.h"

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");

  if (!fp) {
    error("cannot open file: %s", path);
  }

  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  rewind(fp);

  char *buf = malloc(size + 1);

  if (!buf) {
    error("memory allocation failed");
  }

  fread(buf, 1, size, fp);
  buf[size] = '\0';
  fclose(fp);

  return buf;
}

static void print_indent(int depth) {
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
}

void print_ast(Node *node, int depth) {
  if (!node) return;

  print_indent(depth);

  switch (node->kind) {
    case ND_NUM:
      printf("ND_NUM(%d)\n", node->val);
      return;

    case ND_VAR:
      printf("ND_VAR(%s)\n", node->var->name);
      return;

    case ND_FUNCALL:
      printf("ND_FUNCALL(name=%s)\n", node->funcname);
      for (Node *arg = node->args; arg; arg = arg->next) {
        print_ast(arg, depth + 1);
      }
      return;

    case ND_EXPR_STMT:
      printf("ND_EXPR_STMT\n");
      print_ast(node->lhs, depth + 1);
      return;

    case ND_RETURN:
      printf("ND_RETURN\n");
      print_ast(node->lhs, depth + 1);
      return;

    case ND_ADD:
      printf("ND_ADD\n");
      print_ast(node->lhs, depth + 1);
      print_ast(node->rhs, depth + 1);
      return;

    case ND_SUB:
      printf("ND_SUB\n");
      print_ast(node->lhs, depth + 1);
      print_ast(node->rhs, depth + 1);
      return;

    case ND_ASSIGN:
      printf("ND_ASSIGN\n");
      print_ast(node->lhs, depth + 1);
      print_ast(node->rhs, depth + 1);
      return;

    case ND_BLOCK:
      printf("ND_BLOCK\n");
      for (Node *n = node->body; n; n = n->next) {
        print_ast(n, depth + 1);
      }
      return;

    case ND_IF:
      printf("ND_IF\n");
      print_indent(depth + 1);
      printf("Condition:\n");
      print_ast(node->cond, depth + 2);
      print_indent(depth + 1);
      printf("Then:\n");
      print_ast(node->then, depth + 2);
      if (node->els) {
        print_indent(depth + 1);
        printf("Else:\n");
        print_ast(node->els, depth + 2);
      }
      return;

    case ND_FOR:
      printf("ND_FOR\n");
      if (node->init) {
        print_indent(depth + 1);
        printf("Init:\n");
        print_ast(node->init, depth + 2);
      }
      if (node->cond) {
        print_indent(depth + 1);
        printf("Cond:\n");
        print_ast(node->cond, depth + 2);
      }
      if (node->inc) {
        print_indent(depth + 1);
        printf("Inc:\n");
        print_ast(node->inc, depth + 2);
      }
      print_indent(depth + 1);
      printf("Then:\n");
      print_ast(node->then, depth + 2);
      return;

    default:
      print_indent(depth);
      printf("Unknown node kind: %d\n", node->kind);
      return;
  }
}

int main(int argc, char **argv) {
  FILE *out = fopen("main.s", "w");

  if (argc == 2) {
    Token *tok = tokenize(argv[1]);
    Function *prog = parse(tok);
    codegen(prog, out);
  } else {
    char *source = read_file(argv[2]);
    Token *tok = tokenize(source);
    Function *prog = parse(tok);

    // for (Function *fn = prog; fn; fn = fn->next) {
    //   printf("Function: %s\n", fn->name);
    //   print_ast(fn->body, 1);
    // }
    codegen(prog, out);
  }

  fclose(out);

  return 0;
}
