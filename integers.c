#include "math.h"
#include <stdint.h>

#define ASCII_CODE(ch) (uint8_t) ch

#define ASCII_CODE_ZERO ASCII_CODE('0')
#define ASCII_CODE_NINE ASCII_CODE('9')

#define IS_CHAR_NUMBER(ch)  ASCII_CODE(ch) >= ASCII_CODE_ZERO  && ASCII_CODE(ch) <= ASCII_CODE_NINE
#define CHAR_TO_NUM(ch) ASCII_CODE(ch) - ASCII_CODE_ZERO


int getLengthOfStrInt(char *stringInt) {
    int length = 0;
    char *currentSymbol = stringInt;

    for (; IS_CHAR_NUMBER(*currentSymbol) || (*currentSymbol == '-'); currentSymbol++) {
        length++;
    }

    return length;
}

int parseStrIntByLength(char *strInt, int length) {
    char *currentSymbol = strInt;

    int coefficient = 1;
    int result = 0;

    int amountAdditionalZeros = length - 1;

    if (*currentSymbol == '-') {
        coefficient = -1;
        currentSymbol++;
        amountAdditionalZeros--;
        length--;
    }

    for (uint32_t i = 0; i < length; i++) {
        result += (int) (CHAR_TO_NUM(*currentSymbol))
                  *
                  iPow(10, amountAdditionalZeros);

        currentSymbol++;
        amountAdditionalZeros--;
    }

    return result * coefficient;
}

int parseInt(char *string) {
    int length = getLengthOfStrInt(string);
    int integer = parseStrIntByLength(string, length);

    return integer;
}
