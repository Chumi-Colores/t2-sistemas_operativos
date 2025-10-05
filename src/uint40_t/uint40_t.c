#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "uint40_t.h"

uint40_t uint40_from_uint64(uint64_t x) {
    uint40_t n;
    for (int i = 0; i < 5; i++) {
        n.bytes[i] = (x >> (8 * i));
    }
    return n;
}

uint64_t uint64_from_uint40(uint40_t n) {
    uint64_t x = 0;
    for (int i = 0; i < 5; i++) {
        x |= ((uint64_t)n.bytes[i]) << (8 * i);
    }
    return x;
}


void print_binary(const void *data, size_t size) {
    const uint8_t *bytes = (const uint8_t *)data;
    for (int i = (int)size - 1; i >= 0; i--) {
        for (int bit = 7; bit >= 0; bit--) {
            printf("%c", (bytes[i] & (1 << bit)) ? '1' : '0');
        }
        printf(" ");
    }
    printf("\n");
}