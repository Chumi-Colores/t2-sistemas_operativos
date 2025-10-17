#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inverted_page_table_entry.h"


bool get_validity(InvertedPageTableEntry* entry)
{
    return entry->data[2] & 0b00000001;
}

uint16_t get_processesIdentifier(InvertedPageTableEntry* entry)
{
    // PID = bits 1–10 (10 bits)
    uint16_t pid = (((entry->data[1] << 8) | entry->data[2]) & 0b0000011111111110) >> 1;
    return pid; // quitamos los 2 bits más altos no significativos
}

uint16_t get_virtualPageNumber(InvertedPageTableEntry* entry)
{
    // VPN = bits 11–23 (13 bits)
    // Si uno ignora el entry->data[2] son los bits bits 8–15 (13 bits)
    uint16_t vpn = ((entry->data[0] & 0xFE) << 5) | ((entry->data[1] >> 3) & 0x1F);
    return vpn;
}

void set_validity(InvertedPageTableEntry* entry, bool validity)
{
    entry->data[2] = (entry->data[2] & 0b11111110) | (validity &  0b00000001);
}

void set_processesIdentifier(InvertedPageTableEntry* entry, uint16_t pid)
{
    // PID = bits 1–10 (10 bits)
    entry->data[2] = (entry->data[2] & 0b00000001) | ((pid << 1) & 0b11111110);
    entry->data[1] = (entry->data[1] & 0b11111000) | ((pid >> 7) & 0b00000111);
}

void set_virtualPageNumber(InvertedPageTableEntry* entry, uint16_t vpn)
{
    // PID = bits 1–10 (10 bits)
    entry->data[1] = (entry->data[1] & 0b00000111) | ((vpn << 3) & 0b11111000);
    entry->data[0] = (vpn >> 5);
}