#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "os_memory_API.h"
#include "../process_control_block_table/process_control_block_table.h"
#include "../inverted_page_table/inverted_page_table.h"
#include "../frame_bitmap/frame_bitmap.h"
#include "../data/data.h"


// Variables globales declaradas como extern en os_memory_API.c
extern char bin_memory_path[100];
extern ProcessControlBlockTable process_control_block_table;
extern InvertedPageTable inverted_page_table;
extern FrameBitmap frame_bitmap;
extern Data data;

void mount_memory(char* memory_path) {
    // 1. Guardar la ruta en la variable global
    strcpy(bin_memory_path, memory_path);

    // 2. Abrir el archivo binario de memoria física
    FILE* mem_file = fopen(memory_path, "rb");
    if (mem_file == NULL) {
        perror("Error al abrir el archivo de memoria");
        exit(EXIT_FAILURE);
    }

    // 3. Leer la tabla de PCBs (8 KB)
    size_t pcb_table_bytes = sizeof(*process_control_block_table.entries) * process_control_block_table.num_entries;
    printf("pcb_table_bytes: %zu\n", pcb_table_bytes);
    size_t read_bytes = fread(process_control_block_table.entries, 1, pcb_table_bytes, mem_file);
    if (read_bytes != pcb_table_bytes) {
        fprintf(stderr, "Error al leer la tabla de PCBs.\n");
        goto error;
    }

    // 4. Leer tabla invertida de páginas (192 KB)
    size_t inverted_page_table_bytes = sizeof(*inverted_page_table.entries) * inverted_page_table.num_entries;
    printf("inverted_page_table_bytes: %zu\n", inverted_page_table_bytes);
    read_bytes = fread(inverted_page_table.entries, 1, inverted_page_table_bytes, mem_file);
    if (read_bytes != inverted_page_table_bytes) {
        fprintf(stderr, "Error al leer la tabla invertida de páginas.\n");
        goto error;
    }

    // 5. Leer el bitmap (8 KB)
    size_t frame_bitmap_bytes = sizeof(uint8_t) * frame_bitmap.num_bytes;
    printf("frame_bitmap_bytes: %zu\n", frame_bitmap_bytes);
    read_bytes = fread(frame_bitmap.bytes, 1, frame_bitmap_bytes, mem_file);
    if (read_bytes != frame_bitmap_bytes) {
        fprintf(stderr, "Error al leer el bitmap.\n");
        goto error;
    }

    // 6. Leer el área de datos (2 GB)
    size_t data_area_bytes = sizeof(*data.frames) * data.num_frames;
    printf("data_area_bytes: %zu\n", data_area_bytes);
    read_bytes = fread(data.frames, 1, data_area_bytes, mem_file);
    if (read_bytes != data_area_bytes) {
        fprintf(stderr, "Error al leer el área de datos.\n");
        goto error;
    }

    // 7. Cerrar archivo
    fclose(mem_file);
    return;

    error:
        fclose(mem_file);
        exit(EXIT_FAILURE);
}


// // funciones procesos

void list_processes() {
    for (size_t i = 0; i < process_control_block_table.num_entries; i++) {
        ProcessControlBlock* pcb = &process_control_block_table.entries[i];
        if (pcb->state) {
            char name[15];
            memcpy(name, pcb->name, 14);
            name[14] = '\0'; // name no incluye'\0 final'
            printf("%u %s\n", pcb->id, name);
        }
    }
}

int processes_slots() {
    int free_slots = 0;
    for (size_t i = 0; i < process_control_block_table.num_entries; i++) {
        if (!process_control_block_table.entries[i].state) {
            free_slots++;
        }
    }
    return free_slots;
}

void list_files(int process_id) {
    ProcessControlBlock* pcb = get_ProcessControlBlock(&process_control_block_table, process_id);

    if (pcb == NULL) {
        return;
    }

    for (int i = 0; i < 10; i++) {
        osmFile* osm_file = &pcb->file_table[i];
        if (osm_file->validity) {
            char name[15];
            memcpy(name, osm_file->name, 14);
            name[14] = '\0'; // name no incluye'\0 final'
            uint64_t file_size = uint64_from_uint40(osm_file->file_size);
            int vpn = (osm_file->virtual_adress >> 5) & 0xFFF;

            printf("%#x %llu %#x %s\n", vpn, (unsigned long long)file_size, osm_file->virtual_adress, name);
        }
    }
}

void frame_bitmap_status() {
    size_t total_frames = frame_bitmap.num_bytes * 8;
    size_t used = 0;
    for (size_t b = 0; b < frame_bitmap.num_bytes; b++) {
        uint8_t byte = frame_bitmap.bytes[b];
        for (int bit = 0; bit < 8; bit++) {
            used += (((byte >> bit) & 0b00000001) == 1);
        }
    }

    size_t free = total_frames - used;

    printf("USADOS: %zu LIBRES: %zu\n", used, free);
}


// // funciones archivos