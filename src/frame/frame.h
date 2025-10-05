#pragma once
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t bytes[32768]; // almacena los 32 KB = 32768 B = 262144 bits
} frame;