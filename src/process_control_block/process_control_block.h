#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include "../osm_File/osm_File.h"

typedef struct {
    bool state;
    char name[14]; // No incluye el ’\0’ al final
    uint8_t id;
    osmFile file_table[10];
} ProcessControlBlockEntry;