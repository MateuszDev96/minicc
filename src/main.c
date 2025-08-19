#include "minicc.h"

#define INLINE_CODE true

#if INLINE_CODE == true
  int main(int argc, char **argv) {
    if (argc != 2)
      error("%s: invalid number of arguments\n", argv[0]);

    Token *tok = tokenize(argv[1]);
    Function *prog = parse(tok);
    codegen(prog);

    return 0;
  }
#endif

#if INLINE_CODE == false
  char *read_file(char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp)
      error("cannot open file: %s", path);

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
    if (argc != 2) {
      error("%s: invalid number of arguments\n", argv[0]);
    }

    char *source = read_file(argv[1]);
    Token *tok = tokenize(source);
    Function *prog = parse(tok);
    codegen(prog);

    return 0;
  }
#endif
