#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../frame/frame.h"

typedef struct {
    frame* frames;
    size_t num_frames;
} Data;

void initialize_Data(Data* data, size_t num_frames);