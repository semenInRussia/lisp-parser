#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define DEBUG 0
#define MAX_NUMBER_OF_ELEMENTS_FOR_COMPUTE 5

#define ASCII_CODE(ch) (uint8_t) ch

#define ASCII_CODE_ZERO ASCII_CODE('0')
#define ASCII_CODE_NINE ASCII_CODE('9')

#define IS_CHAR_NUMBER(ch)  ASCII_CODE(ch) >= ASCII_CODE_ZERO  && ASCII_CODE(ch) <= ASCII_CODE_NINE
#define CHAR_TO_NUM(ch) ASCII_CODE(ch) - ASCII_CODE_ZERO

int lispToInt(const char *lispExpression) {
    // !!!!! Note: Lisp expression must end  with $
    //       If it not end with $, I not it's not my FAIL

    int result = 0;

    uint8_t openingBrackets = 0;
    uint8_t closingBrackets = 0;

    int elementsForCompute[MAX_NUMBER_OF_ELEMENTS_FOR_COMPUTE];
    int inUseElementsForCompute = 0;

    char *current_symbol = (char *) lispExpression;

    while (*current_symbol != '$' || openingBrackets != closingBrackets) {

#if DEBUG
        printf("%c", *current_symbol);
#endif

        if (*current_symbol == '(') {
            if (openingBrackets > 1) {
                result += lispToInt(current_symbol);
            }
            openingBrackets++;
        } else if (*current_symbol == ')') {
            closingBrackets++;
        } else if (IS_CHAR_NUMBER(*current_symbol)) {
            elementsForCompute[inUseElementsForCompute] = CHAR_TO_NUM(*current_symbol);
            inUseElementsForCompute++;

            result += CHAR_TO_NUM(*current_symbol);
        }

        current_symbol++;
    }

#if DEBUG
    printf("\n%d == %d ???\n", openingBrackets, closingBrackets);
#endif

    assert(openingBrackets == closingBrackets);

    return result;
}


int main() {
    int result = lispToInt("(+ 2 2)$");

#if DEBUG
    printf("result [ %d ]", result);
#endif

    assert(result == 4);

    return 0;
}
