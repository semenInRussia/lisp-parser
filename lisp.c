#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LISP_TESTS 1

typedef enum {
  LISP_TOK_CLOSE,
  LISP_TOK_NUMBER,
  LISP_TOK_OPEN,
  LISP_TOK_OP_ADD,
  LISP_TOK_OP_DIV,
  LISP_TOK_OP_MUL,
  LISP_TOK_OP_SUB,
} LispTokenTyp;

typedef struct {
  LispTokenTyp kind;
  int num;
} LispToken;

typedef enum {
  LISP_ERR_NO_ERROR = 0,
  LISP_ERR_INVALID_OP,
  LISP_ERR_TYPE,
  LISP_ERR_UNCLOSED,
  LISP_ERR_UNDECLARED_SYMBOL,
} LispError;

typedef struct {
  int pos;
  size_t size;
  const char *txt;
  LispError err;
} LispParser;

typedef enum {
  LISP_EXPR_CALL,
  LISP_EXPR_NUMBER,
} LispExprTyp;

typedef enum {
  LISP_OP_ADD,
  LISP_OP_DIV,
  LISP_OP_MUL,
  LISP_OP_SUB,
} LispOp;

struct LispExpr {
  LispExprTyp kind;

  // for call expression (<op> <expr> <expr>), (+ 35 34)
  // or for operation expressions: +, -, *, /
  LispOp op;
  struct LispExpr *a;
  struct LispExpr *b;

  // for number expression <number>, 34, 420
  int num;
};

struct LispExpr *lisp_alloc_expr() {
  struct LispExpr *e = malloc(sizeof(struct LispExpr));
  e->a = e->b = NULL;
  return e;
}

void lisp_free_expr(struct LispExpr *e) {
  if (e == NULL) {
    return;
  }
  lisp_free_expr(e->a);
  lisp_free_expr(e->b);
  free(e);
}

// Tokens Operations

void lisp_error_print(LispError err) {
  switch (err) {
  case LISP_ERR_NO_ERROR:
    assert("SUCCESS! NO ERROR");
  case LISP_ERR_UNDECLARED_SYMBOL:
    printf("UNDECLARED SYMBOL");
    break;
  case LISP_ERR_INVALID_OP:
    printf("INVALID OPERATION");
    break;
  case LISP_ERR_UNCLOSED:
    printf("UNCLOSED EXPRESSION");
    break;
  case LISP_ERR_TYPE:
    printf("TYPE ERROR");
    break;
  }
}

void lisp_token_print(LispToken tok) {
  switch (tok.kind) {
  case LISP_TOK_CLOSE:
    printf("')'");
    break;
  case LISP_TOK_NUMBER:
    printf("'%d'", tok.num);
    break;
  case LISP_TOK_OPEN:
    printf("'('");
    break;
  case LISP_TOK_OP_ADD:
    printf("'+'");
    break;
  case LISP_TOK_OP_DIV:
    printf("'/'");
    break;
  case LISP_TOK_OP_MUL:
    printf("'*'");
    break;
  case LISP_TOK_OP_SUB:
    printf("'-'");
    break;
  }
}

// Parser Operations : lispp

int lisp_p_is_eof(LispParser p) { return (size_t)p.pos >= p.size; }
int lisp_p_is_error(LispParser p) { return p.err > 0; }

char lisp_p_getchar(LispParser p) {
  if (lisp_p_is_eof(p)) {
    assert(0 && "EOF you stupid, check before");
  }
  return p.txt[p.pos];
}

void lisp_p_skip(LispParser *p, int k) { p->pos += k; }

void lisp_p_skip1(LispParser *p) { lisp_p_skip(p, 1); }

void lisp_p_skip_spaces(LispParser *p) {
  while (!lisp_p_is_eof(*p) && isspace(lisp_p_getchar(*p))) {
    lisp_p_skip1(p);
  }
}

int lisp_parse_number(LispParser *p, char buf[256]) {
  if (lisp_p_is_error(*p)) {
    return -1;
  }

  int ans = 0;
  int sign = +1;
  int i = 0;
  if (buf[i] == '-') {
    sign = -1;
    ++i;
  } else if (buf[i] == '+') {
    ++i;
  }

  while (buf[i] != 0 && isdigit(buf[i])) {
    char ch = buf[i];
    ans *= 10;
    ans += ch - '0';
    ++i;
  }

  ans *= sign;

  if (buf[i] != 0) { // we stopped not at the end
    p->err = LISP_ERR_UNDECLARED_SYMBOL;
    return -1;
  }

  return ans;
}

void lisp_p_chop_word(LispParser *p, char buf[256]) {
  if (lisp_p_is_error(*p)) {
    return;
  }

  lisp_p_skip_spaces(p);

  if (lisp_p_is_eof(*p)) {
    p->err = LISP_ERR_UNCLOSED;
    return;
  }

  int i = 0;
  for (i = 0; !lisp_p_is_eof(*p); i++) {
    buf[i] = 0;
    char ch = lisp_p_getchar(*p);
    if (ch == ')' || ch == '(' || isspace(ch)) {
      break;
    }
    buf[i] = ch;
    lisp_p_skip1(p);
  }
  buf[i++] = 0;
}

LispToken lisp_parse_token(LispParser *p) {
  LispToken tok = {0};

  if (lisp_p_is_error(*p)) {
    return tok;
  }

  lisp_p_skip_spaces(p);

  if (lisp_p_is_eof(*p)) {
    p->err = LISP_ERR_UNCLOSED;
    return tok;
  }

  char ch = lisp_p_getchar(*p);

  // [')', '('] - can't be start of symbol
  if (ch == ')' || ch == '(') {
    tok.kind = ch == '(' ? LISP_TOK_OPEN : LISP_TOK_CLOSE;
    lisp_p_skip1(p);
    return tok;
  }

  char buf[256];
  lisp_p_chop_word(p, buf);
  if (lisp_p_is_error(*p)) {
    return tok;
  }

  if (strcmp(buf, "+") == 0) {
    tok.kind = LISP_TOK_OP_ADD;
    return tok;
  }
  if (strcmp(buf, "-") == 0) {
    tok.kind = LISP_TOK_OP_SUB;
    return tok;
  }
  if (strcmp(buf, "*") == 0) {
    tok.kind = LISP_TOK_OP_MUL;
    return tok;
  }
  if (strcmp(buf, "/") == 0) {
    tok.kind = LISP_TOK_OP_DIV;
    return tok;
  }

  tok.kind = LISP_TOK_NUMBER;
  tok.num = lisp_parse_number(p, buf);
  return tok;
}

// continue parsing expression, keeping `LispParser` structure through proccess.
//
// NOTE that every `lisp_p_parse` allocate a memory for expression, so you must
// don't forgoto to call `lisp_free_expr` before exit program
struct LispExpr *lisp_p_parse(LispParser *p) {
  LispToken tok = lisp_parse_token(p);
  struct LispExpr *expr = lisp_alloc_expr();

  if (lisp_p_is_error(*p)) {
    return expr;
  }

  switch (tok.kind) { // ok: '(' or <number>
  case LISP_TOK_CLOSE:
    p->err = LISP_ERR_UNCLOSED;
    return expr;

  case LISP_TOK_OP_ADD:
  case LISP_TOK_OP_DIV:
  case LISP_TOK_OP_MUL:
  case LISP_TOK_OP_SUB:
    // expect number, given operation
    p->err = LISP_ERR_TYPE;
    return expr;

  case LISP_TOK_NUMBER:
    expr->kind = LISP_EXPR_NUMBER;
    expr->num = tok.num;
    return expr;

  case LISP_TOK_OPEN:
    break; // handle below it
  }

  tok = lisp_parse_token(p); // operation
  if (lisp_p_is_error(*p)) {
    return expr;
  }

  switch (tok.kind) {
  case LISP_TOK_CLOSE:
  case LISP_TOK_NUMBER:
  case LISP_TOK_OPEN:
    p->err = LISP_ERR_INVALID_OP;
    return expr;

  case LISP_TOK_OP_ADD:
  case LISP_TOK_OP_DIV:
  case LISP_TOK_OP_MUL:
  case LISP_TOK_OP_SUB:
    expr->kind = LISP_EXPR_CALL;

    LispOp arr[256] = {
        [LISP_TOK_OP_ADD] = LISP_OP_ADD,
        [LISP_TOK_OP_DIV] = LISP_OP_DIV,
        [LISP_TOK_OP_MUL] = LISP_OP_MUL,
        [LISP_TOK_OP_SUB] = LISP_OP_SUB,
    };

    expr->op = arr[tok.kind];

    struct LispExpr *a = lisp_p_parse(p);
    if (lisp_p_is_error(*p)) {
      lisp_free_expr(a);
      return expr;
    }

    struct LispExpr *b = lisp_p_parse(p);
    if (lisp_p_is_error(*p)) {
      lisp_free_expr(a);
      lisp_free_expr(b);
      return expr;
    }

    tok = lisp_parse_token(p);
    if (lisp_p_is_error(*p)) {
      return expr;
    }

    if (tok.kind != LISP_TOK_CLOSE) {
      p->err = LISP_ERR_UNCLOSED;
      return expr;
    }

    expr->a = a;
    expr->b = b;

    return expr;
  }
}

// evaluate parsed lisp expression

int lisp_apply_op(LispOp op, int a, int b) {
  switch (op) {
  case LISP_OP_ADD:
    return a + b;
  case LISP_OP_DIV:
    return a / b;
  case LISP_OP_MUL:
    return a * b;
  case LISP_OP_SUB:
    return a - b;
  }
}

int lisp_eval_expr(struct LispExpr *e, LispError *err) {
  if (*err > 0) {
    return 0;
  }
  switch (e->kind) {
  case LISP_EXPR_CALL:
    LispOp op = e->op;
    int a = lisp_eval_expr(e->a, err);
    if (*err > 0) {
      return -1;
    }
    int b = lisp_eval_expr(e->b, err);
    if (*err > 0) {
      return -1;
    }
    return lisp_apply_op(op, a, b);

  case LISP_EXPR_NUMBER:
    return e->num;
  }
}

// public

int lisp_eval(const char *s, LispError *err) {
  LispParser p = {
      .pos = 0,
      .size = strlen(s),
      .txt = s,
      .err = LISP_ERR_NO_ERROR,
  };
  struct LispExpr *e = lisp_p_parse(&p);
  if (p.err > 0) {
    *err = p.err;
    lisp_free_expr(e);
    return -1;
  }
  int ans = lisp_eval_expr(e, err);
  lisp_free_expr(e);
  return ans;
}

#if LISP_TESTS

#define TEST(body)                                                             \
  printf("%s:%d: info: " #body ": ", __FILE__, __LINE__);                      \
  assert(body);                                                                \
  printf("OK\n");

int main() {
  LispError e = LISP_ERR_NO_ERROR;
  printf("Running tests...\n");

  // Base
  TEST(lisp_eval("(+ 1 2)", &e) == 3);

  // Inner expressions
  TEST(lisp_eval("(+ 1 (* 2 2) )", &e) == 5);

  // All operations in one
  TEST(lisp_eval("(/ (+ 1 (* 2 (- 2 1))) 3)", &e) == 1);

  // Two digits and whitespaces
  TEST(lisp_eval("(+ 22     23)", &e) == 45);

  // Number with sign
  TEST(lisp_eval("(+ +1 -1)", &e) == 0);
}

#else

void usage(const char *program) {
  printf("Usage: %s <s-expression>\n", program);
  printf("Evaluate given lisp s-expression and print the result.");
  printf("\n");
  printf("Lisp language which is implemented in this file support 4 types of "
         "operations\n");
  printf("(+, -, * and /) and 1 type of data: integer\n");
  printf("\n");
  printf("Syntax of lisp is very easy, every statement is s-expression that "
         "have the\n");
  printf("following form: (<op> arg1 arg2 ...), where op is one of +-*/, "
         "arg1 is either\n");
  printf("s-expression or integer\n");
}

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    usage(argv[0]);
    return ENOENT;
  }

  if (strcmp(argv[1], "help") == 0) {
    usage(argv[0]);
    return 0;
  }

  LispError err;
  int x = lisp_eval(argv[1], &err);

  if (err != LISP_ERR_NO_ERROR) {
    printf("error occured: ");
    switch (err) {
    case LISP_ERR_NO_ERROR:
      assert(0 && "unreachable!");
    case LISP_ERR_UNDECLARED_SYMBOL:
      printf("UNDECLARED SYMBOL");
      break;
    case LISP_ERR_INVALID_OP:
      printf("INVALID OPERATION");
      break;
    case LISP_ERR_UNCLOSED:
      printf("UNCLOSED EXPRESSION");
      break;
    }
    return EPERM;
  }

  printf("Result: %d\n", x);
}
#endif
