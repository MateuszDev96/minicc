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

int main(int argc, char **argv) {
  FILE *out = fopen("../main.s", "w");

  if (argc == 2) {
    Token *tok = tokenize(argv[1]);
    Function *prog = parse(tok);
    codegen(prog, out);
  } else {
    char *source = read_file(argv[2]);
    Token *tok = tokenize(source);
    Function *prog = parse(tok);
    codegen(prog, out);
  }

  fclose(out);

  return 0;
}
