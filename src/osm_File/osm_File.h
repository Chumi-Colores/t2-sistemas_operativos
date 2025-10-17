#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include "../uint40_t/uint40_t.h"

typedef struct osmFile {
    bool validity;
    char name[14]; // No incluye el ’\0’ al final
    uint40_t file_size;
    int32_t virtual_adress;
} osmFile;

int get_virtual_page_number_from_virtual_adress(int32_t virtual_adress);