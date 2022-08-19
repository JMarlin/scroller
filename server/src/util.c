#include <util.h>

int decimalDigits(int number) {
    return number < 10 ? 1 : (decimalDigits(number / 10) + 1);
}
