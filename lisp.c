#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// If you need to run test either uncomment next line or compile the program
// with -DLISP_FILES flag
//
// #define LISP_TESTS
//

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

  // parse errors
  LISP_ERR_ARGUMENTS,
  LISP_ERR_INVALID_OP,
  LISP_ERR_TYPE,
  LISP_ERR_UNBALANCED,
  LISP_ERR_UNCLOSED,
  LISP_ERR_UNDECLARED_SYMBOL,

  // runtime errors
  LISP_ERR_ZERO_DIV,
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

// Parser Operations : lispp

int lisp_p_is_eof(LispParser p) { return (size_t)p.pos >= p.size; }
int lisp_p_is_error(LispParser p) { return p.err > 0; }

char lisp_p_getchar(LispParser p) {
  if (lisp_p_is_eof(p)) {
    assert(0 && "EOF you stupid, check before");
  }
  return p.txt[p.pos];
}

void lisp_p_skip(LispParser *p, size_t k) { p->pos += k; }

void lisp_p_skip1(LispParser *p) { lisp_p_skip(p, 1); }

void lisp_p_skip_spaces(LispParser *p) {
  while (!lisp_p_is_eof(*p) && isspace(lisp_p_getchar(*p))) {
    lisp_p_skip1(p);
  }
}

// return 1 => ok
//        0 => false
// read number from buf into x
int lisp_parse_number(char buf[256], int *x) {
  int sign = +1;
  int i = 0;
  if (buf[i] == '-') {
    sign = -1;
    ++i;
  } else if (buf[i] == '+') {
    ++i;
  }

  int ans = 0;
  while (buf[i] != 0 && isdigit(buf[i])) {
    char ch = buf[i];
    ans *= 10;
    ans += ch - '0';
    ++i;
  }

  if (buf[i] != 0) {
    return 0; // bad
  }

  ans *= sign;
  *x = ans;

  return 1; // ok
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

  size_t old_pos = p->pos;
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
  if (!lisp_parse_number(buf, &tok.num)) {
    p->err = LISP_ERR_UNDECLARED_SYMBOL;
    p->pos = old_pos;
    lisp_p_skip_spaces(p);
    return tok;
  }

  return tok;
}

// continue parsing expression, keeping `LispParser` structure through proccess.
//
// NOTE that every `lisp_p_parse` allocate a memory for expression, so you must
// don't forgoto to call `lisp_free_expr` before exit program
struct LispExpr *lisp_p_parse(LispParser *p) {
  size_t old_pos = p->pos;

  LispToken tok = lisp_parse_token(p);
  struct LispExpr *expr = lisp_alloc_expr();

  if (lisp_p_is_error(*p)) {
    return expr;
  }

  switch (tok.kind) { // ok: '(' or <number>
  case LISP_TOK_CLOSE:
    p->err = LISP_ERR_UNBALANCED;
    p->pos = old_pos;
    lisp_p_skip_spaces(p);
    return expr;

  case LISP_TOK_OP_ADD:
  case LISP_TOK_OP_DIV:
  case LISP_TOK_OP_MUL:
  case LISP_TOK_OP_SUB:
    // expect number, given operation
    p->err = LISP_ERR_TYPE;
    p->pos = old_pos;
    lisp_p_skip_spaces(p);
    return expr;

  case LISP_TOK_NUMBER:
    expr->kind = LISP_EXPR_NUMBER;
    expr->num = tok.num;
    return expr;

  case LISP_TOK_OPEN:
    break; // handle below it
  }

  old_pos = p->pos;
  tok = lisp_parse_token(p); // operation
  if (lisp_p_is_error(*p)) {
    return expr;
  }

  switch (tok.kind) {
  case LISP_TOK_CLOSE:
  case LISP_TOK_NUMBER:
  case LISP_TOK_OPEN:
    p->err = LISP_ERR_INVALID_OP;
    p->pos = old_pos;
    lisp_p_skip_spaces(p);
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
      if (p->err == LISP_ERR_UNBALANCED) {
        p->err = LISP_ERR_ARGUMENTS;
      }
      return expr;
    }

    struct LispExpr *b = lisp_p_parse(p);
    if (lisp_p_is_error(*p)) {
      lisp_free_expr(a);
      lisp_free_expr(b);
      if (p->err == LISP_ERR_UNBALANCED) {
        p->err = LISP_ERR_ARGUMENTS;
      }
      return expr;
    }

    old_pos = p->pos;
    tok = lisp_parse_token(p);
    if (lisp_p_is_error(*p)) {
      return expr;
    }

    if (tok.kind != LISP_TOK_CLOSE) {
      p->err = LISP_ERR_ARGUMENTS;
      p->pos = old_pos;
      lisp_p_skip_spaces(p);
      return expr;
    }

    expr->a = a;
    expr->b = b;

    return expr;
  }

  assert(0 && "unreachable!");
  return expr;
}

// evaluate parsed lisp expression

int lisp_apply_op(LispOp op, int a, int b, LispError *err) {
  switch (op) {
  case LISP_OP_ADD:
    return a + b;
  case LISP_OP_DIV:
    if (b == 0) {
      *err = LISP_ERR_ZERO_DIV;
      return 0;
    }
    return a / b;
  case LISP_OP_MUL:
    return a * b;
  case LISP_OP_SUB:
    return a - b;
  }
  assert(0 && "unreachable!");
  return -1;
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
    return lisp_apply_op(op, a, b, err);

  case LISP_EXPR_NUMBER:
    return e->num;
  }
  assert(0 && "unreachable!");
  return -1;
}

// for debuging:

void lisp_op_print(LispOp op) {
  switch (op) {
  case LISP_OP_ADD:
    printf("ADD");
    break;
  case LISP_OP_DIV:
    printf("DIV");
    break;
  case LISP_OP_MUL:
    printf("MUL");
    break;
  case LISP_OP_SUB:
    printf("SUB");
    break;
  }
}

void lisp_expr_print(struct LispExpr *e) {
  switch (e->kind) {
  case LISP_EXPR_CALL:
    lisp_op_print(e->op);
    printf("(");
    lisp_expr_print(e->a);
    printf(", ");
    lisp_expr_print(e->b);
    printf(")");
    break;

  case LISP_EXPR_NUMBER:
    printf("%d", e->num);
    break;
  }
}

void lisp_error_fprint(FILE *f, LispError err) {
  switch (err) {
  case LISP_ERR_NO_ERROR:
    fprintf(f, "SUCCESS! NO ERROR");
    break;
  case LISP_ERR_ARGUMENTS:
    fprintf(f, "ARGUMENTS ERROR, (+ 1 2 3)");
    break;
  case LISP_ERR_UNDECLARED_SYMBOL:
    fprintf(f, "UNDECLARED SYMBOL, (+ a b)");
    break;
  case LISP_ERR_INVALID_OP:
    fprintf(f, "INVALID OPERATION, (1 2 3)");
    break;
  case LISP_ERR_UNCLOSED:
    fprintf(f, "UNCLOSED EXPRESSION, (+ 1 1");
    break;
  case LISP_ERR_TYPE:
    fprintf(f, "TYPE ERROR, (+ + +)");
    break;
  case LISP_ERR_UNBALANCED:
    fprintf(f, "UNBALANCED EXPRESSION, )(");
    break;
  case LISP_ERR_ZERO_DIV:
    fprintf(f, "runtime: DIVISION BY ZERO, 1/0");
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

// public

int lisp_eval(const char *s, LispError *err,
              size_t *pos) { // pos - is where error
  LispParser p = {
      .pos = 0,
      .size = strlen(s),
      .txt = s,
      .err = LISP_ERR_NO_ERROR,
  };
  struct LispExpr *e = lisp_p_parse(&p);
  if (p.err > 0) {
    *err = p.err;
    *pos = p.pos;
    lisp_free_expr(e);
    return -1;
  }
  int ans = lisp_eval_expr(e, err);

  lisp_free_expr(e);
  return ans;
}

void lisp_report_error(const char *program, const char *src, LispError err,
                       size_t pos) {
  const char *label = "    ";
  fprintf(stderr, "%s: error:\n", program);
  fprintf(stderr, "%s%s\n", label, src);
  fprintf(stderr, "%s", label);
  for (size_t i = 0; i < pos; i++) {
    fprintf(stderr, " ");
  }
  fprintf(stderr, "^\n");
  fprintf(stderr, "error: ");
  lisp_error_fprint(stderr, err);
  fprintf(stderr, "\n");
}

#ifdef LISP_TESTS

#define TEST(body)                                                             \
  printf("%s:%d: info: " #body ": ", __FILE__, __LINE__);                      \
  fflush(stdout);                                                              \
  assert(body);                                                                \
  printf("OK\n");                                                              \
  e = 0;

int const1(int t) {
  (void)t;
  return 1;
}

int main() {
  LispError e = LISP_ERR_NO_ERROR;
  size_t pos = 0;
  printf("Running tests...\n");

  // Base
  TEST(lisp_eval("(+ 1 2)", &e, &pos) == 3);

  // Inner expressions
  TEST(lisp_eval("(+ 1 (* 2 2) )", &e, &pos) == 5);

  // All operations in one
  TEST(lisp_eval("(/ (+ 1 (* 2 (- 2 1))) 3)", &e, &pos) == 1);

  // Two digits and whitespaces
  TEST(lisp_eval("(+ 22     23)", &e, &pos) == 45);

  // Number with sign
  TEST(lisp_eval("(+ +1 -1)", &e, &pos) == 0);

  // check on errors:

  // Unclosed expression
  TEST(const1(lisp_eval("(+ (+ 2 2) ", &e, &pos)) && e == LISP_ERR_UNCLOSED);
  TEST(const1(lisp_eval("(+ (+ 2 2 ", &e, &pos)) && e == LISP_ERR_UNCLOSED);

  // Arguments error
  TEST(const1(lisp_eval("(+ 3 3 3)", &e, &pos)) && e == LISP_ERR_ARGUMENTS);
  TEST(const1(lisp_eval("(- 3 (+ 1 2) 3)", &e, &pos)) &&
       e == LISP_ERR_ARGUMENTS);
  TEST(const1(lisp_eval("(+   3)", &e, &pos)) && e == LISP_ERR_ARGUMENTS);
  TEST(const1(lisp_eval("(+)", &e, &pos)) && e == LISP_ERR_ARGUMENTS);

  // Invalid op
  TEST(const1(lisp_eval("(1 2 3)", &e, &pos)) && e == LISP_ERR_INVALID_OP);
  TEST(const1(lisp_eval("()", &e, &pos)) && e == LISP_ERR_INVALID_OP);

  // Undeclared symbol
  TEST(const1(lisp_eval("(himark 2 3)", &e, &pos)) &&
       e == LISP_ERR_UNDECLARED_SYMBOL);
  TEST(const1(lisp_eval("(+ 1 (+ a b))", &e, &pos)) &&
       e == LISP_ERR_UNDECLARED_SYMBOL);

  // Type error
  TEST(const1(lisp_eval("(+ (- 2 2) *)", &e, &pos)) && e == LISP_ERR_TYPE);

  // Unbalanced (unopened)
  TEST(const1(lisp_eval(")", &e, &pos)) && e == LISP_ERR_UNBALANCED);

  // Division by zero
  TEST(const1(lisp_eval("(/ 3 0)", &e, &pos)) && e == LISP_ERR_ZERO_DIV);

  return 0;
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

int interactively_eval(const char *program, const char *src) {
  LispError err = 0;
  size_t pos = 0;
  int x = lisp_eval(src, &err, &pos);

  if (err != LISP_ERR_NO_ERROR) {
    lisp_report_error(program, src, err, pos);
    return EPERM;
  }

  printf("Result: %d\n", x);
  return 0;
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

  return interactively_eval(argv[0], argv[1]);
}
#endif
