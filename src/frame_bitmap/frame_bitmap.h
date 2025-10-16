#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include <stdint.h>

typedef struct {
    uint8_t* bytes;
    size_t num_bytes;
} FrameBitmap;


void initialize_FrameBitmap(FrameBitmap* frame_bitmap, size_t num_bytes);

bool is_frame_free(FrameBitmap* bitmap, size_t frame_number);