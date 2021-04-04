#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "math.h"
#include "integers.h"

#define DEBUG 0

#define NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_INC_CHAR 2

#define ASCII_CODE(ch) (uint8_t) ch

#define ASCII_CODE_ZERO ASCII_CODE('0')
#define ASCII_CODE_NINE ASCII_CODE('9')

#define IS_CHAR_NUMBER(ch)  ASCII_CODE(ch) >= ASCII_CODE_ZERO  && ASCII_CODE(ch) <= ASCII_CODE_NINE

void skipSymbols(char **currentChar, int amountSymbols) {
    (*currentChar) += amountSymbols;
}

void skipSpaces(char **currentChar) {
    while (**currentChar == ' ') {
        (*currentChar)++;
    }
}

int handleSymbol(
        char **currentChar,

        int *result,

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
                return NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_INC_CHAR;
            } else {
                skipSymbols(currentChar, 1);
            }
            break;
        default:
            if (IS_CHAR_NUMBER(**currentChar)) {
                uint32_t lengthOfInteger = getLengthOfStrInt(*currentChar);
                *result += parseStrIntByLength(*currentChar, lengthOfInteger);

                skipSymbols(currentChar, (int) (lengthOfInteger));
            } else {
                skipSymbols(currentChar, 1);
            }
    }

    return 0;
}


int getAmountOfSymbolsFromCurrentSymbolToClosingBracket(char* currentSymbol) {
    int amountOfSymbols = 0;

    for (; *currentSymbol != ')'; currentSymbol++) {
        amountOfSymbols++;
    }

    return amountOfSymbols;
}

int lispToInt(const char *lispExpression) {
/* | input:
     !!!!! Note: Lisp expression must end  with $
           If it not end with $, I not it's not my FAIL
*/

    int result = 0;

    uint32_t openingBrackets = 0;
    uint32_t closingBrackets = 0;

    char *current_symbol = (char *) lispExpression;

    while (((openingBrackets != closingBrackets) || (openingBrackets == 0)) && (*current_symbol != '$')) {
        if (handleSymbol(
                &current_symbol,
                &result,
                &openingBrackets, &closingBrackets
        ) == NEED_ADD_LISP_EXPRESSION_TO_RESULT_AND_INC_CHAR) {
            result += lispToInt(current_symbol);
            current_symbol += getAmountOfSymbolsFromCurrentSymbolToClosingBracket(current_symbol);
        }
    }

#if DEBUG
    printf("%d == %d?\n", openingBrackets, closingBrackets);
#endif

    assert(openingBrackets == closingBrackets);

    return result;
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

    int lispResult = lispToInt("(+ 200 (+ 100 50) (+ 100 100 150 100) 1000 600 600 (+ 1995 5))");

#if DEBUG
    printf("result of LISP expression is [ %d ]\n", lispResult);
#endif

    assert(lispResult == 5000);

    return 0;
}
