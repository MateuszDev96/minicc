#include "minicc.h"

// Maksymalny rozmiar typu int, który obsługujemy (np. 64 bajty = 512 bitów)
#define MAX_INT_SIZE 64

// Cache typów całkowitych, indeks = rozmiar w bajtach
static Type *ty_ints[MAX_INT_SIZE + 1] = {0};

Type *new_type(TypeKind kind, int size);

// Funkcja zwraca typ całkowity o rozmiarze 'size' bajtów
Type *int_type(int size) {
  if (size <= 0 || size > MAX_INT_SIZE) {
    error("invalid int size: %d", size);
  }

  if (ty_ints[size]) {
    return ty_ints[size];
  }

  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_INT;
  ty->size = size;

  ty_ints[size] = ty;
  return ty;
}

bool is_integer(Type *ty) {
  return ty->kind == TY_INT;
}

Type *copy_type(Type *ty) {
  Type *ret = calloc(1, sizeof(Type));
  *ret = *ty;

  return ret;
}

Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->size = 8;  // zakładamy 64-bitowe wskaźniki
  ty->base = base;

  return ty;
}

Type *func_type(Type *return_ty) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_FUNC;
  ty->return_ty = return_ty;

  return ty;
}

Type *array_of(Type *base, int len) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->size = base->size * len;
  ty->base = base;
  ty->array_len = len;

  return ty;
}

void add_type(Node *node) {
  if (!node || node->ty)
    return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);

  for (Node *n = node->body; n; n = n->next)
    add_type(n);

  for (Node *n = node->args; n; n = n->next)
    add_type(n);

  switch (node->kind) {
    case ND_ADD:
    case ND_SUB: {
      Type *lt = node->lhs->ty;
      Type *rt = node->rhs->ty;

      if (lt->kind == TY_PTR && is_integer(rt)) {
        node->ty = lt;
        return;
      }

      if (is_integer(lt) && rt->kind == TY_PTR) {
        node->ty = rt;
        return;
      }

      if (is_integer(lt) && is_integer(rt)) {
        node->ty = copy_type(lt);  // wybierz typ lhs
        return;
      }

      if (lt->kind == TY_PTR && rt->kind == TY_PTR && node->kind == ND_SUB) {
        // typ ptrdiff_t – ten sam rozmiar co int wejściowy
        node->ty = new_type(TY_INT, lt->base->size);
        return;
      }

      error_tok(node->tok, "invalid operands");
    }

    case ND_MUL:
    case ND_DIV:
    case ND_NEG:
      node->ty = node->lhs->ty;
      return;

    case ND_ASSIGN:
      if (node->lhs->ty->kind == TY_ARRAY)
        error_tok(node->lhs->tok, "not an lvalue");
      node->ty = node->lhs->ty;
      return;

    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_NUM:
    case ND_FUNCALL:
      node->ty = new_type(TY_INT, 4); // Można zastąpić przez dynamiczną detekcję – np. max(lhs, rhs)
      return;

    case ND_VAR:
      node->ty = node->var->ty;
      return;

    case ND_ADDR:
      if (node->lhs->ty->kind == TY_ARRAY)
        node->ty = pointer_to(node->lhs->ty->base);
      else
        node->ty = pointer_to(node->lhs->ty);
      return;

    case ND_DEREF:
      if (!node->lhs->ty->base)
        error_tok(node->tok, "invalid pointer dereference");
      node->ty = node->lhs->ty->base;
      return;

    default:
      return;
  }
}

