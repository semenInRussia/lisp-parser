#define NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_SKIP_TO_CLOSING_BRACKET_RET_CODE  1
#define UNDEFINED_SYMBOL_IN_LISP_EXPRESSION_RET_CODE -1
#define UNDEFINED_OPERATION_IN_LISP_EXPRESSION_RET_CODE -2
#define NOT_VALID_AMOUNT_OF_BRACKETS_IN_LISP_EXPRESSION_RET_CODE -3


int lispToInt(const char *lispExpression, int *pointerOnRetCode);