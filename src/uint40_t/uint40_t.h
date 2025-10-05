#pragma once
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t bytes[5]; // almacena los 40 bits
} uint40_t;


uint40_t uint40_from_uint64(uint64_t x);


uint64_t uint64_from_uint40(uint40_t n);


void print_binary(const void *data, size_t size);