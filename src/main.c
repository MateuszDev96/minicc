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
  if (!buf)
    error("memory allocation failed");

  fread(buf, 1, size, fp);
  buf[size] = '\0';
  fclose(fp);
  return buf;
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    error("%s: invalid number of arguments\n", argv[0]);
  }

  bool is_string_input = false;
  char *input = NULL;

  if (argc == 3 && strcmp(argv[1], "-s") == 0) {
    is_string_input = true;
    input = argv[2];  // kod źródłowy bezpośrednio z argumentu
  } else if (argc == 2) {
    input = argv[1];  // ścieżka do pliku
  } else {
    error("Usage: %s [-s source_code] | [file_path]\n", argv[0]);
  }

  if (is_string_input) {
    Token *tok = tokenize(input);
    Function *prog = parse(tok);
    codegen(prog);
  } else {
    char *source = read_file(input);
    Token *tok = tokenize(source);
    Function *prog = parse(tok);
    codegen(prog);
  }

  return 0;
}