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
  LISP_ERR_UNCLOSED,
  LISP_ERR_UNDECLARED_SYMBOL,
} LispError;

typedef struct {
  int pos;
  size_t size;
  const char *txt;
  LispError err;
} LispParser;

// Tokens Operations

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

int lisp_apply_op(LispTokenTyp op, int a, int b) {
  switch (op) {
  case LISP_TOK_CLOSE:
  case LISP_TOK_NUMBER:
  case LISP_TOK_OPEN: {
    LispToken t = {.kind = op, .num = 0};
    printf("Given a ");
    lisp_token_print(t);
    printf("\n");
    fflush(stdout);
    assert(0 && "check apply first parameter: token, it must be +-*/");
    return -1;
  }
  case LISP_TOK_OP_ADD:
    return a + b;
  case LISP_TOK_OP_DIV:
    return a / b;
  case LISP_TOK_OP_MUL:
    return a * b;
  case LISP_TOK_OP_SUB:
    return a - b;
  }
}

// Parser Operations : lispp

int lisp_p_is_eof(LispParser p) { return p.pos >= p.size; }
int lisp_p_is_error(LispParser p) { return p.err > 0; }

char lisp_p_getchar(LispParser p) {
  if (lisp_p_is_eof(p)) {
    assert(0 && "EOF you stupid, check before");
  }
  return p.txt[p.pos];
}

void lisp_p_skip(LispParser *p, int k) { p->pos += k; }

inline void lisp_p_skip1(LispParser *p) { lisp_p_skip(p, 1); }

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
  }

  if (buf[i] == '+') {
    ++i;
  }

  while (buf[i] != 0 && isdigit(buf[i])) {
    char ch = buf[i];
    ans *= 10;
    ans += ch - '0';
    ++i;
  }

  ans *= sign;

  if (buf[i] != 0) { // we stop not at the end
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
  LispToken tok;

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

int lisp_p_parse(LispParser *p) {
  LispToken tok = lisp_parse_token(p);
  if (lisp_p_is_error(*p)) {
    return -1;
  }

  switch (tok.kind) { // ok: '(' or <number>
  case LISP_TOK_CLOSE:
  case LISP_TOK_OP_ADD:
  case LISP_TOK_OP_DIV:
  case LISP_TOK_OP_MUL:
  case LISP_TOK_OP_SUB:
    p->err = LISP_ERR_UNCLOSED;
    return -1;

  case LISP_TOK_NUMBER:
    return tok.num;

  case LISP_TOK_OPEN:
    break; // handle below it
  }

  tok = lisp_parse_token(p); // operation
  if (lisp_p_is_error(*p)) {
    return -1;
  }

  switch (tok.kind) {
  case LISP_TOK_CLOSE:
  case LISP_TOK_NUMBER:
  case LISP_TOK_OPEN:
    p->err = LISP_ERR_INVALID_OP;
    return -1;

  case LISP_TOK_OP_ADD:
  case LISP_TOK_OP_DIV:
  case LISP_TOK_OP_MUL:
  case LISP_TOK_OP_SUB:
    int a = lisp_p_parse(p);
    if (lisp_p_is_error(*p)) {
      return -1;
    }

    int b = lisp_p_parse(p);
    if (lisp_p_is_error(*p)) {
      return -1;
    }

    LispTokenTyp knd = tok.kind;
    tok = lisp_parse_token(p);
    if (lisp_p_is_error(*p)) {
      return -1;
    }

    if (tok.kind != LISP_TOK_CLOSE) {
      p->err = LISP_ERR_UNCLOSED;
      return -1;
    }

    return lisp_apply_op(knd, a, b);
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
  int ans = lisp_p_parse(&p);
  *err = p.err;
  return ans;
}

#if LISP_TESTS

#define TEST(body)                                                             \
  {                                                                            \
    printf("%s:%d: info: " #body ": ", __FILE__, __LINE__);                    \
    assert(body);                                                              \
    printf("OK\n");                                                            \
  }

int main() {
  LispError e;
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
