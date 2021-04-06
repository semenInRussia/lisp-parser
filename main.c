#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "integers.h"

#define DEBUG 1
#define MAX_ELEMENTS_FOR_COMPUTE 10

#define NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_SKIP_TO_CLOSING_BRACKET 1

#define ASCII_CODE(ch) (uint8_t) ch
#define ASCII_CODE_ZERO ASCII_CODE('0')

#define ASCII_CODE_NINE ASCII_CODE('9')

#define IS_CHAR_NUMBER(ch)  ASCII_CODE(ch) >= ASCII_CODE_ZERO  && ASCII_CODE(ch) <= ASCII_CODE_NINE

#define PLUS_CHAR '+'

#define SUB_CHAR '-'

#define MULTIPLICATION_CHAR '*'

#define DIVISION_CHAR '/'

typedef int (*operation_t)(const int[MAX_ELEMENTS_FOR_COMPUTE], uint32_t);


void skipSymbols(char **currentChar, int amountSymbols) {
    (*currentChar) += amountSymbols;
}

void skipSpaces(char **currentChar) {
    while (**currentChar == ' ') {
        (*currentChar)++;
    }
}

int getAmountOfSymbolsFromCurrentSymbolToClosingBracket(char *currentSymbol) {
    int amountOfSymbols = 0;

    for (; *currentSymbol != ')'; currentSymbol++) {
        amountOfSymbols++;
    }

    return amountOfSymbols;
}

void skipSymbolsToClosingBracket(char **currentChar) {
    skipSymbols(
            currentChar,
            getAmountOfSymbolsFromCurrentSymbolToClosingBracket(*currentChar)
    );
}

void parseIntAndAddToArray(char **currentChar, int *pointerOnArray, int *inUseElementsOfArray) {
    int lengthOfInteger = getLengthOfStrInt(*currentChar);

    *(pointerOnArray + (*inUseElementsOfArray)) = parseStrIntByLength(
            *currentChar,
            lengthOfInteger
    );

    (*inUseElementsOfArray)++;

    skipSymbols(currentChar, lengthOfInteger);
}

int iSum(const int *numbersForCompute, uint32_t length) {
    int result = 0;

    for (uint32_t i = 0; i < length; i++) {
        result += numbersForCompute[i];
    }

    return result;
}

int iSub(const int *numbersForCompute, uint32_t length) {
    int decreasing = numbersForCompute[0];
    int subtraction = iSum(&(numbersForCompute[1]), length - 1);

    return decreasing - subtraction;
}

int iMult(const int *numbersForCompute, uint32_t length) {
    int result = 1;

    for (uint32_t i = 0; i < length; i++) {
        result *= numbersForCompute[i];
    }

    return result;
}

int iDiv(const int numbersForCompute[MAX_ELEMENTS_FOR_COMPUTE], uint32_t length) {
    int divisor = numbersForCompute[0];
    int divisible = iSum(&(numbersForCompute[1]), length);

    return divisor / divisible;
}

void handleOperation(
        char **currentChar,
        operation_t *pointerOnOperation
) {

    switch (**currentChar) {
        case PLUS_CHAR:
            *pointerOnOperation = &iSum;
            break;
        case SUB_CHAR:
            *pointerOnOperation = &iSub;
            break;
        case MULTIPLICATION_CHAR:
            *pointerOnOperation = &iMult;
            break;
        case DIVISION_CHAR:
            *pointerOnOperation = &iDiv;
            break;
    }

    skipSymbols(currentChar, 1);
}

int handleSymbol(
        char **currentChar,

        int *numbersForCompute,
        int *inUseNumbersForCompute,

        operation_t *pointerOnOperation,

        uint32_t *openingBrackets,
        uint32_t *closingBrackets
) {
    switch (**currentChar) {
        case ' ':
            skipSpaces(currentChar);
            break;
        case ')':
            (*closingBrackets)++;
            skipSymbols(currentChar, 1);
            break;
        case '(':
            (*openingBrackets)++;

            if (*openingBrackets > 1) {
                return NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_SKIP_TO_CLOSING_BRACKET;
            } else {
                skipSymbols(currentChar, 1); // Skip 1 Open Bracket: '('
                skipSpaces(currentChar);
                handleOperation(currentChar, pointerOnOperation);
            }
            break;
        default:
            if (IS_CHAR_NUMBER(**currentChar) || **currentChar == '-') {
                parseIntAndAddToArray(currentChar, numbersForCompute, inUseNumbersForCompute);
            } else {
                // Undefined symbol...
                skipSymbols(currentChar, 1);
            }
            break;
    }

    return 0;
}

int lispToInt(const char *lispExpression) {
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

    while (((openingBrackets != closingBrackets) || (openingBrackets == 0)) && (*current_symbol != '$')) {
        if (handleSymbol(
                &current_symbol,
                numbersForCompute, &inUseNumbersForCompute,
                &operation,
                &openingBrackets, &closingBrackets
        ) == NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_SKIP_TO_CLOSING_BRACKET) {

            numbersForCompute[inUseNumbersForCompute] = lispToInt(current_symbol);
            inUseNumbersForCompute++;
            skipSymbolsToClosingBracket(&current_symbol);

        }
    }

#if DEBUG
    printf("%d == %d?\n", openingBrackets, closingBrackets);
#endif

    assert(openingBrackets == closingBrackets);

    return (*operation)(numbersForCompute, (uint32_t) inUseNumbersForCompute);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("You're not enter a LISP expression!!!");
        exit(EINVAL);
    }

    char* lispExpression = argv[1];

    printf("Result is:\n %d", lispToInt(lispExpression));

    return 0;
}