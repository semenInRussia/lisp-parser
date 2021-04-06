#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "math.h"
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

typedef int (*operation_t) (const int[MAX_ELEMENTS_FOR_COMPUTE], uint32_t);


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

int sum(const int numbersForCompute[MAX_ELEMENTS_FOR_COMPUTE], uint32_t length) {
    int result = 0;

    for (uint32_t i = 0; i < length; i++) {
        result += numbersForCompute[i];
    }

    return result;
}

int sub(const int numbersForCompute[MAX_ELEMENTS_FOR_COMPUTE], uint32_t length) {
    int decreasing = numbersForCompute[0];
    int subtraction = sum(&(numbersForCompute[1]), length - 1);

    return decreasing - subtraction;
}

int mult(const int numbersForCompute[MAX_ELEMENTS_FOR_COMPUTE], uint32_t length) {
    int result = 1;

    for (uint32_t i = 0; i < length; i++) {
        result *= numbersForCompute[i];
    }

    return result;
}

int div(const int numbersForCompute[MAX_ELEMENTS_FOR_COMPUTE], uint32_t length) {
    int divisor = numbersForCompute[0];
    int divisible = sum(&(numbersForCompute[1]), length);

    return divisor / divisible;
}

void handleOperation(
        char **currentChar,
        operation_t *pointerOnOperation
) {

    switch (**currentChar) {
        case PLUS_CHAR:
            *pointerOnOperation = &sum;
            break;
        case SUB_CHAR:
            *pointerOnOperation = &sub;
            break;
        case MULTIPLICATION_CHAR:
            *pointerOnOperation = &mult;
            break;
        case DIVISION_CHAR:
            *pointerOnOperation = &div;
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


int main() {
    int powerRes = iPow(10, 2);

#if DEBUG
    printf("power result [ %d ]\n", powerRes);
#endif

    assert(powerRes == 100);

    int parsedInt = parseInt("1001 ");
    int parsedIntByLength = parseStrIntByLength("1001 ", 4);
    uint32_t lengthOfInteger = getLengthOfStrInt("1001 ");

#if DEBUG
    printf("parsed integer is [ %d ]\n", parsedInt);
    printf("length of integer is [ %d ]\n", lengthOfInteger);
    printf("parsed integer by length is [ %d ]\n", parsedIntByLength);
#endif

    assert(parsedInt == 1001);
    assert(lengthOfInteger == 4);
    assert(parsedIntByLength == 1001);

    int negativeParsedInt = parseInt("-100 ");
    int lengthOfNegativeParsedInt = getLengthOfStrInt("-100 ");

#if DEBUG
    printf("Negative parsed int is: [ %d ]\n", negativeParsedInt);
    printf("length of Negative parsed int is: [ %d ]\n", lengthOfNegativeParsedInt);
#endif

    assert(lengthOfNegativeParsedInt == 4);
    assert(negativeParsedInt == -100);

    int lispResult = lispToInt("(* 3 2 1)");

#if DEBUG
    printf("result of LISP expression is [ %d ]\n", lispResult);
#endif

    assert(lispResult == 6);

    return 0;
}