#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include "../osm_File/osm_File.h"

typedef struct ProcessControlBlock {
    bool state;
    char name[13]; // No incluye el ’\0’ al final
    uint8_t id;
    osmFile file_table[10];
    
} ProcessControlBlock;