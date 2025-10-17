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
    // Bit 23 = MSB de data[2] según get_validity
    if (validity)
        entry->data[2] |= 0b10000000;  // 1000 0000
    else
        entry->data[2] &= 0b01111111;  // 0111 1111
}

void set_processesIdentifier(InvertedPageTableEntry* entry, uint16_t pid)
{
    // PID son 10 bits útiles (0..9), alineado con get_processesIdentifier
    // Combinar con los bits existentes de data[2..0] excepto los PID
    uint32_t raw =
        ((uint32_t)entry->data[2] << 16) |
        ((uint32_t)entry->data[1] << 8) |
        (uint32_t)entry->data[0];

    // Limpiar bits 22..13 (PID)
    raw &= ~((uint32_t)0x3FF << 13);

    // Insertar PID
    raw |= ((uint32_t)(pid & 0x3FF) << 13);

    // Guardar de nuevo en data[]
    entry->data[2] = (raw >> 16) & 0xFF;
    entry->data[1] = (raw >> 8) & 0xFF;
    entry->data[0] = raw & 0xFF;
}

void set_virtualPageNumber(InvertedPageTableEntry* entry, uint16_t vpn)
{
    // VPN 12 bits útiles (0..11), almacenado en bits 12..0
    uint32_t raw =
        ((uint32_t)entry->data[2] << 16) |
        ((uint32_t)entry->data[1] << 8) |
        (uint32_t)entry->data[0];

    // Limpiar bits 12..0
    raw &= ~0x1FFF;

    // Insertar VPN
    raw |= ((uint32_t)(vpn & 0x0FFF) | 0x1000); // agrega el bit no significativo como 1 si quieres simular padding como en get

    // Guardar de nuevo en data[]
    entry->data[2] = (raw >> 16) & 0xFF;
    entry->data[1] = (raw >> 8) & 0xFF;
    entry->data[0] = raw & 0xFF;
}