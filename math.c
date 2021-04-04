
int iPow(int base, int pow) {
    int result = 1;

    for (int i = 0; i < pow; i++) {
        result *= base;
    }

    return result;
}