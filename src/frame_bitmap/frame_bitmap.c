#include "stdbool.h"
#include "stdlib.h"
#include <stdint.h>
#include "frame_bitmap.h"

void initialize_FrameBitmap(FrameBitmap* frame_bitmap, size_t num_bytes) {
    frame_bitmap->bytes = malloc(sizeof(uint8_t) * num_bytes);
    frame_bitmap->num_bytes = num_bytes;
}


bool is_frame_free(FrameBitmap* frame_bitmap, size_t frame_number) {
    size_t byte_index = frame_number >> 3;
    size_t bit_index = frame_number & 0b111;

    if (byte_index >= frame_bitmap->num_bytes) {
        return false; // Fuera de los límites del bitmap
    }

    return (frame_bitmap->bytes[byte_index] & (1 << bit_index)) == 0;
}