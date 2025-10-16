#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../frame/frame.h"
#include "data.h"

void initialize_Data(Data* data, size_t num_frames) {
    data->frames = malloc(sizeof(frame) * num_frames);
    data->num_frames = num_frames;
}