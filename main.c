#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lispParser.h"

#define STRING_FOR_PROMPT "lisp"
#define USER_EXIT_CODE 1
#define MAX_BUFFER_SIZE 80

#define END_OF_INPUT '\n'

#define printWarning(str) printf("[WARNING!] %s!!!\n\n\n", str)


void readUserMessage(char message[MAX_BUFFER_SIZE]) {
    uint32_t currentSymbolIndex = 0;
    char currentChar = '1';   // By start currentChar not must equal a END_OF_INPUTS

    while (currentChar != END_OF_INPUT) {
        currentChar = (char) getchar();

        message[currentSymbolIndex] = currentChar;
        currentSymbolIndex++;
    }

    message[currentSymbolIndex - 1] = '\0';
}


void printHelp() {
    printf("Hello, world and my friend YOU!\n"
           "This program may be parse a lisp expressions\n"
           "\nJust enter LISP EXPRESSION for parsing.\n"
           "Enter \"/help\" for getting help information\n"
           "Enter \"/exit\" for exit from program.\n\n"
           "GOOD LUCK!!\n");
}

void printPrompt() {
    printf("%s>", STRING_FOR_PROMPT);
}

void handleRetCodeOfLispToInt(int retCode) {
    switch (retCode) {
        case (UNDEFINED_OPERATION_IN_LISP_EXPRESSION_RET_CODE):
            printWarning("Undefined operation in LISP expression");
            break;
        case UNDEFINED_SYMBOL_IN_LISP_EXPRESSION_RET_CODE:
            printWarning("Undefined symbol in LISP expression");
            break;
        case NOT_VALID_AMOUNT_OF_BRACKETS_IN_LISP_EXPRESSION_RET_CODE:
            printWarning("You are not valid enter brackets");
            break;
        default: ;
    }
}

errno_t handleMessage(char *message) {
    if (strcmp("/exit", message) == 0) {
        printf("BY, FRIEND!\n");

        return USER_EXIT_CODE;
    }

    else if (strcmp("/help", message) == 0) {
        printHelp();
    }

    else if (strcmp("", message) != 0) {
        int retCode = 0;

        printf("%s is equal:\n\t%d\n",
               message,
               lispToInt(message, &retCode)
        );

        handleRetCodeOfLispToInt(retCode);
    }

    return EXIT_SUCCESS;
}

void runREPL() {
    printHelp();

    while (1) {
        printPrompt();

        char message[MAX_BUFFER_SIZE];
        readUserMessage(message);

        if (handleMessage(message) == USER_EXIT_CODE) {
            exit(0);
        }
    }


}


int main(int argc, char *argv[]) {
    if (argc == 1) {
        runREPL();
        exit(0);
    }

    char *lispExpression = argv[1];

    int retCode;

    printf("Result is:\n %d", lispToInt(lispExpression, &retCode));

    handleRetCodeOfLispToInt(retCode);

    return 0;
}