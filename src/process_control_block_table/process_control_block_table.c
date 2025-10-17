#include "stdbool.h"
#include "stdlib.h"
#include "../process_control_block/process_control_block.h"
#include "process_control_block_table.h"

void initialize_ProcessControlBlockTable(ProcessControlBlockTable* table, size_t num_entries) {
    table->entries = malloc(sizeof(ProcessControlBlock) * num_entries);
    table->num_entries = num_entries;
}

ProcessControlBlock* get_ProcessControlBlock(ProcessControlBlockTable* table, int process_id) {
    for (size_t i = 0; i < table->num_entries; i++) {
        if (table->entries[i].state && table->entries[i].id == (uint8_t)process_id) {
            return &table->entries[i];
        }
    }
    return NULL;
}