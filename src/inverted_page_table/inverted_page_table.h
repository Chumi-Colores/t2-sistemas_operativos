#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include "../inverted_page_table_entry/inverted_page_table_entry.h"

typedef struct {
    InvertedPageTableEntry* entries;
    size_t num_entries;
} InvertedPageTable;


void initialize_InvertedPageTable(InvertedPageTable* table, size_t num_entries);

InvertedPageTableEntry* get_InvertedPageTableEntry(InvertedPageTable* table, int process_id, int virtual_page_number);