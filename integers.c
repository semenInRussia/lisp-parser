#include "math.h"
#include <stdint.h>

#define ASCII_CODE(ch) (uint8_t) ch

#define ASCII_CODE_ZERO ASCII_CODE('0')
#define ASCII_CODE_NINE ASCII_CODE('9')

#define IS_CHAR_NUMBER(ch)  ASCII_CODE(ch) >= ASCII_CODE_ZERO  && ASCII_CODE(ch) <= ASCII_CODE_NINE
#define CHAR_TO_NUM(ch) ASCII_CODE(ch) - ASCII_CODE_ZERO


uint32_t getLengthOfStrInt(char *stringInt) {
    uint32_t length = 0;
    char *currentSymbol = stringInt;

    for (; IS_CHAR_NUMBER(*currentSymbol); currentSymbol++) {
        length++;
    }

    return length;
}

int parseStrIntByLength(char *strInt, uint32_t length) {
    uint32_t amountAdditionalZeros = length - 1;
    char *currentSymbol = strInt;

    int result = 0;

    for (uint32_t i = 0; i < length; i++) {
        result += (int) (CHAR_TO_NUM(*currentSymbol))
                  *
                  iPow(10, (int) amountAdditionalZeros);

        currentSymbol++;
        amountAdditionalZeros--;
    }

    return result;
}

int parseInt(char *string) {
    unsigned int length = getLengthOfStrInt(string);
    int integer = parseStrIntByLength(string, length);

    return integer;
}
