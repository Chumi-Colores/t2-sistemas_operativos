#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
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
    size_t read_bytes = fread(process_control_block_table.entries, 1, pcb_table_bytes, mem_file);
    if (read_bytes != pcb_table_bytes) {
        fprintf(stderr, "Error al leer la tabla de PCBs.\n");
        goto error;
    }

    // 4. Leer tabla invertida de páginas (192 KB)
    size_t inverted_page_table_bytes = sizeof(*inverted_page_table.entries) * inverted_page_table.num_entries;
    read_bytes = fread(inverted_page_table.entries, 1, inverted_page_table_bytes, mem_file);
    if (read_bytes != inverted_page_table_bytes) {
        fprintf(stderr, "Error al leer la tabla invertida de páginas.\n");
        goto error;
    }

    // 5. Leer el bitmap (8 KB)
    size_t frame_bitmap_bytes = sizeof(uint8_t) * frame_bitmap.num_bytes;
    read_bytes = fread(frame_bitmap.bytes, 1, frame_bitmap_bytes, mem_file);
    if (read_bytes != frame_bitmap_bytes) {
        fprintf(stderr, "Error al leer el bitmap.\n");
        goto error;
    }

    // 6. Leer el área de datos (2 GB)
    size_t data_area_bytes = sizeof(*data.frames) * data.num_frames;
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
void write_process_in_bin(ProcessControlBlock* pcb_entry) {
    int n = pcb_entry->id;
    FILE* mem_file = fopen(bin_memory_path, "r+b");
    long start = 0L;
    long offset = start + (sizeof(*pcb_entry) * n);  // posición donde escribir
    fseek(mem_file, offset, SEEK_SET);
    fwrite(pcb_entry, sizeof(*pcb_entry), 1, mem_file);
    fclose(mem_file);
}

int start_process(int process_id, char* process_name) {
    ProcessControlBlock* pcb_entry = &process_control_block_table.entries[process_id];
    
    if (pcb_entry->state) {
        return -1;
    }

    pcb_entry->state = 1;
    pcb_entry->id = process_id;
    strncpy(pcb_entry->name, process_name, 14);
    write_process_in_bin(pcb_entry);
    return 0;
}

void write_frame_from_bitmap_in_bin(int frame_number) {
    FILE* mem_file = fopen(bin_memory_path, "r+b");
    long start = 8192L + 196608L; // inicio del bitmap
    long offset = start + (sizeof(uint8_t) * frame_number / 8);
    fseek(mem_file, offset, SEEK_SET);
    uint8_t byte = frame_bitmap.bytes[frame_number / 8];
    fwrite(&byte, sizeof(uint8_t), 1, mem_file);
    fclose(mem_file);
}

void free_frame_in_bitmap(int frame_number) {
    frame_bitmap.bytes[frame_number / 8] &= ~(1 << (frame_number % 8));
    write_frame_from_bitmap_in_bin(frame_number);
}

void write_entry_from_inverted_page_table_in_bin(InvertedPageTableEntry* entry, int entry_index) {
    FILE* mem_file = fopen(bin_memory_path, "r+b");
    long start = 8192L;
    long offset = start + (sizeof(*entry) * entry_index);  // posición donde escribir
    fseek(mem_file, offset, SEEK_SET);
    fwrite(entry, sizeof(*entry), 1, mem_file);
    fclose(mem_file);
}

void free_entry_from_inverted_page_table(int process_id, int virtual_page_number) {
    int entry_index = get_InvertedPageTableEntryIndex(&inverted_page_table, process_id, virtual_page_number);
    if (entry_index == -1) {
        return;
    }
    InvertedPageTableEntry* entry = &inverted_page_table.entries[entry_index];
    set_validity(entry, 0);

    write_entry_from_inverted_page_table_in_bin(entry, entry_index);

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
    pcb_entry->state = 0;

    write_process_in_bin(pcb_entry);
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

void save_file_entry_to_bin(int pcb_index, int file_index) {
    FILE* mem_file = fopen(bin_memory_path, "r+b");
    if (!mem_file) {
        perror("No se pudo abrir el archivo de memoria para escritura");
        exit(EXIT_FAILURE);
    }
    
    size_t pcb_offset = sizeof(ProcessControlBlock) * pcb_index;
    size_t file_offset = file_index * sizeof(osmFile);

    fseek(mem_file, pcb_offset + offsetof(ProcessControlBlock, file_table) + file_offset, SEEK_SET);
    fwrite(&process_control_block_table.entries[pcb_index].file_table[file_index], sizeof(osmFile), 1, mem_file);

    fclose(mem_file);
}

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
            save_file_entry_to_bin(get_ProcessControlBlockIndex(&process_control_block_table, process_id), i);
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

int allocate_free_frame(int pid, int vpn) {
    size_t total_frames = frame_bitmap.num_bytes * 8;

    for (size_t pfn = 0; pfn < total_frames; pfn++) {
        size_t byte_index = pfn / 8;
        uint8_t bit_index = pfn % 8;

        if (!(frame_bitmap.bytes[byte_index] & (1 << bit_index))) {

            frame_bitmap.bytes[byte_index] |= (1 << bit_index);
            write_frame_from_bitmap_in_bin(pfn); // Persistir


            InvertedPageTableEntry* entry = &inverted_page_table.entries[pfn];
            set_validity(entry, true);
            set_processesIdentifier(entry, pid);
            set_virtualPageNumber(entry, vpn);
            write_entry_from_inverted_page_table_in_bin(entry, pfn);

            return (int)pfn; // Devuelve el frame asignado
        }
    }

    return -1; // No hay frames libres
}



int write_file(osmFile* file_desc, char* src) {
    if (!file_desc || !file_desc->validity) {
        return -1;
    }

    // Abrir archivo fuente en el sistema local
    FILE* input_file = fopen(src, "rb");
    if (!input_file) {
        printf("Error abriendo archivo %s", src);
        return -1;
    }

    // Determinar PID al que pertenece file_desc
    int pid = -1;
    for (int i = 0; i < (int)process_control_block_table.num_entries; i++) {
        ProcessControlBlock* pcb = &process_control_block_table.entries[i];
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
        fclose(input_file);
        return -1;
    }

    // Descomponer dirección virtual inicial del archivo
    uint32_t vaddr = (uint32_t)file_desc->virtual_adress;
    uint32_t vpn = get_virtual_page_number_from_virtual_adress(vaddr);
    uint32_t offset = vaddr & 0x7FFF; // 15 bits

    uint64_t total_written = 0;
    uint8_t buffer[sizeof(frame)];
    long data_offset = process_control_block_table.num_entries * sizeof(ProcessControlBlock) + inverted_page_table.num_entries * sizeof(InvertedPageTableEntry) + frame_bitmap.num_bytes * sizeof(uint8_t);
    FILE* mem_file = fopen(bin_memory_path, "r+b");

    while (true) {
        // Leer del archivo fuente
        size_t read_bytes = fread(buffer, 1, sizeof(buffer), input_file);
        if (read_bytes == 0) break; // fin de archivo

        size_t buffer_offset = 0;

        while (buffer_offset < read_bytes) {
            // Obtener/crear frame para la VPN actual
            int pfn = get_InvertedPageTableEntryIndex(&inverted_page_table, pid, vpn);
            if (pfn == -1) {
                // No existe entrada → buscar frame libre y asignarlo
                pfn = allocate_free_frame(pid, vpn);
                if (pfn == -1) {
                    fprintf(stderr, "No quedan frames disponibles\n");
                    fclose(input_file);
                    fclose(mem_file);
                    return (int)total_written;
                }
            }

            // Cuántos bytes caben en este frame desde el offset actual
            size_t space_in_frame = sizeof(frame) - offset;
            size_t to_write = read_bytes - buffer_offset;
            if (to_write > space_in_frame) to_write = space_in_frame;

            // Copiar datos al frame
            memcpy(&data.frames[pfn].bytes[offset], &buffer[buffer_offset], to_write);
            long frame_start = (long)pfn * sizeof(frame);
            fseek(mem_file, data_offset + frame_start + offset, SEEK_SET);
            fwrite(&buffer[buffer_offset], 1, to_write, mem_file);
            total_written += to_write;
            buffer_offset += to_write;
            offset += to_write;

            // Si llené el frame, paso al siguiente VPN
            if (offset == sizeof(frame)) {
                vpn++;
                offset = 0;
            }
        }

        if (read_bytes < sizeof(buffer)) break; // fin de archivo
    }

    fclose(input_file);
    fclose(mem_file);

    // Actualizar file_desc->file_size
    file_desc->file_size = uint40_from_uint64(total_written);

    // Guardar en memoria persistente (binario)
    int pcb_index = get_ProcessControlBlockIndex(&process_control_block_table, pid);
    for (int i = 0; i < FILES_PER_PROCESS; i++) {
        if (&process_control_block_table.entries[pcb_index].file_table[i] == file_desc) {
            save_file_entry_to_bin(pcb_index, i);
            break;
        }
    }

    return (int)total_written;
}


void delete_file(int process_id, char* file_name)
{
    ProcessControlBlock* pcb = get_ProcessControlBlock(&process_control_block_table, process_id);
    if (!pcb) return;

    for (int i = 0; i < FILES_PER_PROCESS; i++) {
        if (pcb->file_table[i].validity && strncmp(pcb->file_table[i].name, file_name, 14) == 0) {
            pcb->file_table[i].file_size = uint40_from_uint64(0);
            int32_t virtual_adress = pcb->file_table[i].virtual_adress;
            int vpn = get_virtual_page_number_from_virtual_adress(virtual_adress);

            free_entry_from_inverted_page_table(process_id, vpn);
            return;
        }
    }
}

void close_file(osmFile* file_desc)
{
    if (!file_desc) return;

    file_desc->validity = false;


    for (size_t i = 0; i < process_control_block_table.num_entries; i++)
    {
        ProcessControlBlock* pcb = &process_control_block_table.entries[i];
        if (!pcb->state) continue;
        for (size_t j = 0; j < FILES_PER_PROCESS; j++)
        {
            if (&pcb->file_table[j] == file_desc)
            {
                save_file_entry_to_bin(i, j);
                return;
            }
        }
    }
}