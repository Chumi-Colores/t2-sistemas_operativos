#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inverted_page_table_entry.h"


bool get_validity(InvertedPageTableEntry* entry)
{
    // bit 23 = MSB de data[2]
    return (entry->data[2] & 0x80) != 0;
}

uint16_t get_processesIdentifier(InvertedPageTableEntry* entry)
{
    // Extraer los 10 bits de PID (bits 22..13)
    uint32_t raw =
        ((uint32_t)entry->data[2] << 16) |
        ((uint32_t)entry->data[1] << 8) |
        (uint32_t)entry->data[0];

    // Desplazar 13 bits a la derecha → deja los bits 22..13 en posición 9..0
    uint16_t pid_with_padding = (raw >> 13) & 0x3FF;

    // Quitar los 2 bits no significativos (bits 9..8)
    uint16_t pid = pid_with_padding & 0xFF;  // 8 bits útiles

    return pid;
}

uint16_t get_virtualPageNumber(InvertedPageTableEntry* entry)
{
    // Extraer los 13 bits de VPN (bits 12..0)
    uint32_t raw =
        ((uint32_t)entry->data[2] << 16) |
        ((uint32_t)entry->data[1] << 8) |
        (uint32_t)entry->data[0];

    uint16_t vpn_with_padding = raw & 0x1FFF; // 13 bits

    // Quitar el primer bit no significativo (bit 12)
    uint16_t vpn = vpn_with_padding & 0x0FFF; // 12 bits útiles

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