#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include "../process_control_block/process_control_block.h"

typedef struct {
    ProcessControlBlock* entries;
    size_t num_entries;
} ProcessControlBlockTable;

int get_free_ProcessControlBlock_index(ProcessControlBlockTable* table);

void initialize_ProcessControlBlockTable(ProcessControlBlockTable* table, size_t num_entries);


ProcessControlBlock* get_ProcessControlBlock(ProcessControlBlockTable* table, int process_id);


int get_ProcessControlBlockIndex(ProcessControlBlockTable* table, int process_id);