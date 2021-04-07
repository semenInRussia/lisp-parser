#include <stdint.h>

#include "integers.h"

#define DEBUG 0
#define MAX_ELEMENTS_FOR_COMPUTE 100

#define NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_SKIP_TO_CLOSING_BRACKET_RET_CODE  1
#define UNDEFINED_SYMBOL_IN_LISP_EXPRESSION_RET_CODE -1
#define UNDEFINED_OPERATION_IN_LISP_EXPRESSION_RET_CODE -2
#define NOT_VALID_AMOUNT_OF_BRACKETS_IN_LISP_EXPRESSION_RET_CODE -3

#define ASCII_CODE(ch) (uint8_t) ch

#define ASCII_CODE_ZERO ASCII_CODE('0')

#define ASCII_CODE_NINE ASCII_CODE('9')

#define IS_CHAR_NUMBER(ch)  (ASCII_CODE(ch) >= ASCII_CODE_ZERO)  && (ASCII_CODE(ch) <= ASCII_CODE_NINE)
#define SIZE_OF_OPERATION_CODE 1
#define SUM_OPERATION_CODE '+'
#define DIFFERENCE_OPERATION_CODE '-'

#define MULTIPLICATION_OPERATION_CODE '*'

#define DIVISION_OPERATION_CODE '/'

typedef int (*operation_t)(const int[MAX_ELEMENTS_FOR_COMPUTE], uint32_t);

void skipSymbols(char **pointerOnPointerOnCurrentChar, int amountSymbols) {
    (*pointerOnPointerOnCurrentChar) += amountSymbols;
}

void skipSpaces(char **pointerOnPointerOnCurrentChar) {
    while (**pointerOnPointerOnCurrentChar == ' ') {
        skipSymbols(pointerOnPointerOnCurrentChar, 1);
    }
}

int getAmountOfSymbolsFromCurrentSymbolToClosingBracket(char *pointerOnCurrentSymbol) {
    int amountOfSymbols = 0;

    while (*pointerOnCurrentSymbol != ')') {
        amountOfSymbols++;
        pointerOnCurrentSymbol++;
    }

    return amountOfSymbols;
}

void skipSymbolsToClosingBracket(char **pointerOnPointerOnCurrentChar) {
    skipSymbols(pointerOnPointerOnCurrentChar,
                getAmountOfSymbolsFromCurrentSymbolToClosingBracket(*pointerOnPointerOnCurrentChar));
}

void parseIntAndAddToArray(char **pointerOnPointerOnCurrentChar,
                           int array[],
                           int *pointerOnInUseElementsOfArray
) {
    int lengthOfInteger = getLengthOfStrInt(*pointerOnPointerOnCurrentChar);

    array[*pointerOnInUseElementsOfArray] = parseStrIntByLength(
            *pointerOnPointerOnCurrentChar,
            lengthOfInteger
    );

    (*pointerOnInUseElementsOfArray)++;

    skipSymbols(pointerOnPointerOnCurrentChar, lengthOfInteger);
}

int sum(const int *numbersForCompute, uint32_t length) {
    int result = 0;

    for (uint32_t i = 0; i < length; i++) {
        result += numbersForCompute[i];
    }

    return result;
}

int difference(const int *numbersForCompute, uint32_t length) {
    int decreasing = numbersForCompute[0];
    int subtraction = sum(&(numbersForCompute[1]), length - 1);

    return decreasing - subtraction;
}

int multiplication(const int *numbersForCompute, uint32_t length) {
    int result = 1;

    for (uint32_t i = 0; i < length; i++) {
        result *= numbersForCompute[i];
    }

    return result;
}

int division(const int *numbersForCompute, uint32_t length) {
    int divisor = numbersForCompute[0];
    int divisible = sum(&(numbersForCompute[1]), length);

    return divisor / divisible;
}

/*
 * INPUT:
     ...
 * OUTPUT:
     Set pointerOnPointerOnChar, when set a position of parsing.
     Set pointerOnOperation, when parse operation, depending on string for handling.
*/
void handleOperation(
        char **pointerOnCurrentOperationCode,
        operation_t *pointerOnOperation,
        int *pointerOnRetCode
) {

    switch (**pointerOnCurrentOperationCode) {
        case SUM_OPERATION_CODE:
            *pointerOnOperation = &sum;
            break;
        case DIFFERENCE_OPERATION_CODE:
            *pointerOnOperation = &difference;
            break;
        case MULTIPLICATION_OPERATION_CODE:
            *pointerOnOperation = &multiplication;
            break;
        case DIVISION_OPERATION_CODE:
            *pointerOnOperation = &division;
            break;
        default:
            *pointerOnRetCode = UNDEFINED_OPERATION_IN_LISP_EXPRESSION_RET_CODE;
            break;
    }

    skipSymbols(pointerOnCurrentOperationCode, SIZE_OF_OPERATION_CODE);
}


/*
 * INPUT:
     ...
 * OUTPUT:
     Set pointerOnPointerOnChar, when set a position of parsing.
     Set pointerOnOperation, when parse operation, depending on string for handling.
     Set pointerOnInUseNumbersForCompute & numbersForCompute, when handle number in string.
     Set pointerOpeningBrackets & pointerClosingBrackets, when handle a brackets in string.

     Return 0 as Success. Also return
     NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_SKIP_TO_CLOSING_BRACKET_RET_CODE.
*/
void handleSymbol(
        char **pointerOnPointerOnCurrentChar,

        int *numbersForCompute,
        int *pointerOnUnUseNumbersForCompute,

        operation_t *pointerOnOperation,

        uint32_t *pointerOnOpeningBrackets,
        uint32_t *pointerOnClosingBrackets,

        int *pointerOnRetCode
) {
    switch (**pointerOnPointerOnCurrentChar) {
        case ' ':
            skipSpaces(pointerOnPointerOnCurrentChar);
            break;
        case ')':
            (*pointerOnClosingBrackets)++;
            skipSymbols(pointerOnPointerOnCurrentChar, 1);
            break;
        case '(':
            (*pointerOnOpeningBrackets)++;

            if (*pointerOnOpeningBrackets > 1) {
                *pointerOnRetCode = NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_SKIP_TO_CLOSING_BRACKET_RET_CODE;
            } else {
                skipSymbols(pointerOnPointerOnCurrentChar, 1); // Skip Open Bracket '('
                skipSpaces(pointerOnPointerOnCurrentChar);

                handleOperation(pointerOnPointerOnCurrentChar, pointerOnOperation, pointerOnRetCode);
            }
            break;
        default:
            if (IS_CHAR_NUMBER(**pointerOnPointerOnCurrentChar) ||
                **pointerOnPointerOnCurrentChar == '-') {

                parseIntAndAddToArray(
                        pointerOnPointerOnCurrentChar,
                        numbersForCompute,
                        pointerOnUnUseNumbersForCompute
                );

            } else {
                *pointerOnRetCode = UNDEFINED_SYMBOL_IN_LISP_EXPRESSION_RET_CODE;

                skipSymbols(pointerOnPointerOnCurrentChar, 1);
            }
            break;
    }
}

int lispToInt(const char *lispExpression, int *pointerOnRetCode) {
/* | input:
     [!!!] Note: If Lisp expression not valid, then it must end  with $
           If it not end with $, then program go in INFINITY LOOP,
           and it's not my FAIL
*/

    uint32_t openingBrackets = 0;
    uint32_t closingBrackets = 0;

    int numbersForCompute[MAX_ELEMENTS_FOR_COMPUTE] = {0};
    int inUseNumbersForCompute = 0;

    int (*operation)(const int *, uint32_t);

    char *current_symbol = (char *) lispExpression;

    while (((openingBrackets != closingBrackets) || (openingBrackets == 0)) &&
           (*current_symbol != '$' && *current_symbol != '\0')) {
        handleSymbol(
                &current_symbol,
                numbersForCompute, &inUseNumbersForCompute,
                &operation,
                &openingBrackets, &closingBrackets,
                pointerOnRetCode
        );

        switch (*pointerOnRetCode) {
            case (NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_SKIP_TO_CLOSING_BRACKET_RET_CODE): {

                numbersForCompute[inUseNumbersForCompute] =
                        lispToInt(current_symbol, pointerOnRetCode);
                inUseNumbersForCompute++;
                skipSymbolsToClosingBracket(&current_symbol);
                break;
            }
            case (UNDEFINED_OPERATION_IN_LISP_EXPRESSION_RET_CODE):
                return 0;
        }
    }

#if DEBUG
    printf("%d == %d?\n", openingBrackets, closingBrackets);
#endif

    if (closingBrackets != openingBrackets) {
        *pointerOnRetCode = NOT_VALID_AMOUNT_OF_BRACKETS_IN_LISP_EXPRESSION_RET_CODE;
    }

    return (*operation)(numbersForCompute, (uint32_t) inUseNumbersForCompute);
}
