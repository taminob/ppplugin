#include <stdio.h>

void initialize(void) {
    puts("C initialize");
}

int loop(int value) {
    printf("C: %d\n", value);
    return value - 2;
}
