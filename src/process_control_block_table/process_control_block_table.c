#include "stdbool.h"
#include "stdlib.h"
#include "../process_control_block/process_control_block.h"
#include "process_control_block_table.h"

void initialize_ProcessControlBlockTable(ProcessControlBlockTable* table, size_t num_entries) {
    table->entries = malloc(sizeof(ProcessControlBlock) * num_entries);
    table->num_entries = num_entries;
}