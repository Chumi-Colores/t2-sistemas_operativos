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
extern const int FILES_PER_PROCESS;

// funciones generales
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
            int vpn = get_virtual_page_number_from_virtual_adress(osm_file->virtual_adress);

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


/* ====== FUNCIONES PARA PROCESOS ====== */

int start_process(int process_id, char* process_name) {
    ProcessControlBlock* pcb_entry = &process_control_block_table.entries[process_id];
    
    if (pcb_entry->state) {
        return -1;
    }

    pcb_entry->state = 1;
    pcb_entry->id = process_id;
    strncpy(pcb_entry->name, process_name, 14);
    return 0;
}

void free_frame_in_bitmap(int frame_number) {
    frame_bitmap.bytes[frame_number / 8] &= ~(1 << (frame_number % 8));
}

void free_entry_from_inverted_page_table(int process_id, int virtual_page_number) {
    int entry_index = get_InvertedPageTableEntryIndex(&inverted_page_table, process_id, virtual_page_number);
    if (entry_index == -1) {
        return;
    }
    InvertedPageTableEntry* entry = &inverted_page_table.entries[entry_index];
    set_validity(entry, 0);

    // Liberar el frame en el bitmap
    free_frame_in_bitmap(entry_index);
}

int free_everything_from_process(int process_id) {
    ProcessControlBlock* pcb_entry = &process_control_block_table.entries[process_id];

    for (size_t i = 0; i < 10; i++) {
        pcb_entry->file_table[i].validity = 0;
        int32_t virtual_adress = pcb_entry->file_table[i].virtual_adress;
        int vpn = get_virtual_page_number_from_virtual_adress(virtual_adress);
        free_entry_from_inverted_page_table(process_id, vpn);
    }
    return 0;
}

int finish_process(int process_id) {
    ProcessControlBlock* pcb_entry = &process_control_block_table.entries[process_id];
    
    if (!pcb_entry->state) {
        return -1;
    }

    return free_everything_from_process(process_id);
}

int clear_all_processes() {
    int terminated_processes_count = 0;
    for (size_t i = 0; i < process_control_block_table.num_entries; i++) {
        ProcessControlBlock* pcb_entry = &process_control_block_table.entries[i];
        if (pcb_entry->state) {
            terminated_processes_count++;
            free_everything_from_process(i);
        }
    }
    return terminated_processes_count;
}

int file_table_slots(int process_id) {
    ProcessControlBlock* pcb_entry = &process_control_block_table.entries[process_id];
    if (!pcb_entry->state) {
        return -1;
    }

    int free_slots = 0;
    for (size_t i = 0; i < 10; i++) {
        if (pcb_entry->file_table[i].validity == 0) {
            free_slots++;
        }
    }
    return free_slots;
}


/* ====== FUNCIONES PARA ARCHIVOS ====== */

osmFile* open_file(int process_id, char* file_name, char mode) {
    ProcessControlBlock* pcb = get_ProcessControlBlock(&process_control_block_table, process_id);
    if (!pcb) return NULL;

    for (int i = 0; i < FILES_PER_PROCESS; i++) {
        if (pcb->file_table[i].validity && strncmp(pcb->file_table[i].name, file_name, 14) == 0) {
            if (mode == 'r') {
                return &pcb->file_table[i];
            } else {
                // No se puede abrir en 'w' si ya existe
                return NULL;
            }
        }
    }

    if (mode == 'w') {
        for (int i = 0; i < FILES_PER_PROCESS; i++) {
            if (pcb->file_table[i].validity) {
                continue;
            }
            strncpy(pcb->file_table[i].name, file_name, 14);

            // Inicializar campos
            pcb->file_table[i].validity = true;
            pcb->file_table[i].file_size = uint40_from_uint64(0);

            // Asignar dirección virtual inicial
            for (int vpn = 0; vpn < 4096; vpn++) {
                InvertedPageTableEntry* entry = get_InvertedPageTableEntry(&inverted_page_table, pcb->id, vpn);
                
                if (!entry || !get_validity(entry)) {
                    pcb->file_table[i].virtual_adress = vpn << 15;
                    break;
                }
            }
            return &pcb->file_table[i];
        }
    }

    return NULL;
}


int read_file(osmFile* file_desc, char* dest) {
    if (!file_desc || !file_desc->validity) {
        return -1;
    }

    // Abrir archivo local de salida
    FILE* out = fopen(dest, "wb");
    if (!out) {
        perror("Error abriendo archivo destino");
        return -1;
    }

    // Tamaño real del archivo en bytes
    uint64_t file_size = uint64_from_uint40(file_desc->file_size);
    if (file_size == 0) {
        fclose(out);
        return 0;
    }

    // Buscar pid asociado al file_desc
    int pid = -1;
    for (int i = 0; i < (int)process_control_block_table.num_entries; i++) {
        ProcessControlBlock* pcb = &process_control_block_table.entries[i];

        if (!pcb) continue;
        if (!pcb->state) continue;

        for (int j = 0; j < FILES_PER_PROCESS; j++) {
            if (&pcb->file_table[j] == file_desc) {
                pid = pcb->id;
                break;
            }
        }
        if (pid != -1) break;
    }

    if (pid == -1) {
        fprintf(stderr, "Error: no se encontró PID para el osmFile*\n");
        fclose(out);
        return -1;
    }

    // Descomponer dirección virtual inicial
    uint32_t vaddr = (uint32_t)file_desc->virtual_adress;
    uint32_t vpn = get_virtual_page_number_from_virtual_adress(vaddr);   // 12 bits VPN
    printf("Starting VPN: %#x\n", vpn);
    uint32_t offset = vaddr & 0x7FFF;      // 15 bits offset

    uint64_t bytes_remaining = file_size;
    uint64_t total_written = 0;

    while (bytes_remaining > 0) {
        // Obtener PFN (índice de frame) para esa entrada
        int pfn = get_InvertedPageTableEntryIndex(&inverted_page_table, pid, vpn);
        if (pfn == -1) {
            fprintf(stderr, "Error: PFN %d fuera de rango\n", pfn);
            fclose(out);
            return -1;
        }

        // Puntero al inicio del frame + offset
        uint8_t* src_ptr = &data.frames[pfn].bytes[offset];

        // Cuántos bytes quedan en este frame desde el offset
        size_t chunk = sizeof(frame) - offset;
        if (chunk > bytes_remaining) chunk = (size_t)bytes_remaining;

        // Escribir directamente al archivo destino
        size_t written = fwrite(src_ptr, 1, chunk, out);
        if (written != chunk) {
            perror("Error escribiendo al archivo destino");
            fclose(out);
            return -1;
        }

        total_written += written;
        bytes_remaining -= written;

        // Avanzar a la siguiente página/frame
        vpn++;
        offset = 0; // solo el primer frame puede tener offset != 0
    }

    fclose(out);
    return (int)total_written; // devuelve bytes leídos/escritos al archivo destino
}


int write_file(osmFile* file_desc, char* src)
{
    return -1;
}

void delete_file(int process_id, char* file_name)
{
    ProcessControlBlock* pcb = get_ProcessControlBlock(&process_control_block_table, process_id);
    if (!pcb) return;

    for (int i = 0; i < FILES_PER_PROCESS; i++) {
        if (pcb->file_table[i].validity && strncmp(pcb->file_table[i].name, file_name, 14) == 0) {
            pcb->file_table[i].validity = 0;
            int32_t virtual_adress = pcb->file_table[i].virtual_adress;
            int vpn = get_virtual_page_number_from_virtual_adress(virtual_adress);

            free_entry_from_inverted_page_table(process_id, vpn);
            return;
        }
    }
}

void close_file(osmFile* file_desc)
{
    return;
}