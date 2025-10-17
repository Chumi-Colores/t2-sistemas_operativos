#include "stdbool.h"
#include "stdlib.h"
#include "../inverted_page_table_entry/inverted_page_table_entry.h"
#include "inverted_page_table.h"
#include "../uint40_t/uint40_t.h"

void initialize_InvertedPageTable(InvertedPageTable* table, size_t num_entries) {
    table->entries = malloc(sizeof(InvertedPageTableEntry) * num_entries);
    table->num_entries = num_entries;
}


InvertedPageTableEntry* get_InvertedPageTableEntry(InvertedPageTable* table, int process_id, int virtual_page_number) {
    for (size_t i = 0; i < table->num_entries; i++) {
        InvertedPageTableEntry* entry = &table->entries[i];
        if (process_id == get_processesIdentifier(entry) && virtual_page_number == get_virtualPageNumber(entry)) {
            return entry;
        }
    }
    return NULL; // No se encontró la entrada
}


int get_InvertedPageTableEntryIndex(InvertedPageTable* table, int process_id, int virtual_page_number) {
    for (size_t i = 0; i < table->num_entries; i++) {
        InvertedPageTableEntry* entry = &table->entries[i];
        if (process_id == get_processesIdentifier(entry) && virtual_page_number == get_virtualPageNumber(entry)) {
            return i;
        }
    }
    return -1; // No se encontró la entrada
}