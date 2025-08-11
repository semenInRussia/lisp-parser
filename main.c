#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_CLOSE,
  TOKEN_NUMBER,
  TOKEN_OPEN,
  TOKEN_OP_ADD,
  TOKEN_OP_DIV,
  TOKEN_OP_MUL,
  TOKEN_OP_SUB,
} TokenKind;

typedef struct {
  TokenKind kind;
  int num;
} Token;

void print_token(Token tok) {
  switch (tok.kind) {
  case TOKEN_CLOSE:
    printf("')'");
    break;
  case TOKEN_NUMBER:
    printf("'%d'", tok.num);
    break;
  case TOKEN_OPEN:
    printf("'('");
    break;
  case TOKEN_OP_ADD:
    printf("'+'");
    break;
  case TOKEN_OP_DIV:
    printf("'/'");
    break;
  case TOKEN_OP_MUL:
    printf("'*'");
    break;
  case TOKEN_OP_SUB:
    printf("'-'");
    break;
  }
}

int apply(TokenKind op, int a, int b) {
  switch (op) {
  case TOKEN_CLOSE:
  case TOKEN_NUMBER:
  case TOKEN_OPEN: {
    Token t = {.kind = op, .num = 0};
    printf("Given a ");
    print_token(t);
    printf("\n");
    fflush(stdout);
    assert(0 && "check apply first parameter: token, it must be +-*/");
    return -1;
  }
  case TOKEN_OP_ADD:
    return a + b;
  case TOKEN_OP_DIV:
    return a / b;
  case TOKEN_OP_MUL:
    return a * b;
  case TOKEN_OP_SUB:
    return a - b;
  }
}

typedef enum {
  ERR_NO_ERROR = 0,
  ERR_INVALID_OP,
  ERR_UNBALANCED,
  ERR_UNCLOSED,
  ERR_UNDECLARED_SYMBOL,
} Error;

typedef struct {
  int pos;
  size_t len;
  char *txt;
  Error err;
} Parser;

int parser_is_eof(Parser p) { return p.pos >= p.len; }

char parser_getchar(Parser p) {
  if (parser_is_eof(p)) {
    assert(0 && "EOF you stupid, check before");
  }
  return p.txt[p.pos];
}

void skip(Parser *p, int k) { p->pos += k; }

void skip1(Parser *p) { skip(p, 1); }

void skip_whitespaces(Parser *p) {
  while (!parser_is_eof(*p) && isspace(parser_getchar(*p))) {
    skip1(p);
  }
}

int parse_number(Parser *p, char buf[256]) {
  int ans = 0;
  int sign = 1;
  int i = 0;
  if (buf[i] == '-') {
    sign = -1;
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
    p->err = ERR_UNDECLARED_SYMBOL;
    return -1;
  }

  return ans;
}

void parser_chop_word(Parser *p, char buf[256]) {
  skip_whitespaces(p);

  if (parser_is_eof(*p)) {
    p->err = ERR_UNBALANCED;
    return;
  }

  int i = 0;
  for (i = 0; !parser_is_eof(*p); i++) {
    buf[i] = 0;
    char ch = parser_getchar(*p);
    if (ch == ')' || ch == '(' || isspace(ch)) {
      break;
    }
    buf[i] = ch;
    skip1(p);
  }
  buf[i++] = 0;
}

Token parse_token(Parser *p) {
  skip_whitespaces(p);

  Token tok;

  if (parser_is_eof(*p)) {
    p->err = ERR_UNBALANCED;
    return tok;
  }

  if (p->err > 0) {
    return tok;
  }

  char ch = parser_getchar(*p);

  // [')', '('] - can't be start of symbol
  if (ch == ')' || ch == '(') {
    tok.kind = ch == '(' ? TOKEN_OPEN : TOKEN_CLOSE;
    skip1(p);
    return tok;
  }

  char buf[256];
  parser_chop_word(p, buf);

  if (strcmp(buf, "+") == 0) {
    tok.kind = TOKEN_OP_ADD;
    return tok;
  }
  if (strcmp(buf, "-") == 0) {
    tok.kind = TOKEN_OP_SUB;
    return tok;
  }
  if (strcmp(buf, "*") == 0) {
    tok.kind = TOKEN_OP_MUL;
    return tok;
  }
  if (strcmp(buf, "/") == 0) {
    tok.kind = TOKEN_OP_DIV;
    return tok;
  }

  tok.kind = TOKEN_NUMBER;
  tok.num = parse_number(p, buf);
  return tok;
}

int parser_eval(Parser *p) {
  Token tok = parse_token(p);
  if (p->err > 0) {
    return -1;
  }

  switch (tok.kind) {
  case TOKEN_CLOSE:
  case TOKEN_OP_ADD:
  case TOKEN_OP_DIV:
  case TOKEN_OP_MUL:
  case TOKEN_OP_SUB:
    p->err = ERR_UNBALANCED;
    return -1;

  case TOKEN_NUMBER:
    return tok.num;

  case TOKEN_OPEN:
    break; // handle below it
  }

  tok = parse_token(p); // operation

  switch (tok.kind) {
  case TOKEN_CLOSE:
  case TOKEN_NUMBER:
  case TOKEN_OPEN:
    p->err = ERR_INVALID_OP;
    return -1;

  case TOKEN_OP_ADD:
  case TOKEN_OP_DIV:
  case TOKEN_OP_MUL:
  case TOKEN_OP_SUB:
    int a = parser_eval(p);
    int b = parser_eval(p);
    TokenKind knd = tok.kind;
    tok = parse_token(p);
    if (tok.kind != TOKEN_CLOSE) {
      p->err = ERR_UNCLOSED;
      return -1;
    }
    return apply(knd, a, b);
  }
}

int eval(char *s, Error *err_code) {
  Parser p = {
      .pos = 0,
      .len = strlen(s),
      .txt = s,
      .err = ERR_NO_ERROR,
  };
  int ans = parser_eval(&p);
  *err_code = p.err;
  return ans;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <s-expression>\n", argv[0]);
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
    return 1;
  }

  Error err_code;
  int x = eval(argv[1], &err_code);

  if (err_code != ERR_NO_ERROR) {
    printf("error occured ");
    switch (err_code) {
    case ERR_NO_ERROR:
      assert(0 && "unreachable!");
    case ERR_UNBALANCED:
      printf("UNBALANCED EXPRESSION");
      break;
    case ERR_UNDECLARED_SYMBOL:
      printf("UNDECLARED SYMBOL");
      break;
    case ERR_INVALID_OP:
      printf("INVALID OPERATION");
      break;
    case ERR_UNCLOSED:
      printf("UNCLOSED EXPRESSION");
      break;
    }
    return 1;
  }

  printf("Result:\n");
  printf("  %d", x);
}
