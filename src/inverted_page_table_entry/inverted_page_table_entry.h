#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"

typedef struct {
    unsigned char data[3];
    // Estructura de bits (de LSB a MSB):
    // data[2]: VPPPPPPP   (1 bit validez + 7 bits de PID)
    // data[1]: PPPNNNNN   (3 bits de PID + 5 bits de VPN)
    // data[0]: NNNNNNNN   (8 bits de VPN)
} InvertedPageTableEntry;

bool get_validity(InvertedPageTableEntry* inverted_page_table_entry);

uint16_t get_virtualPageNumber(InvertedPageTableEntry* inverted_page_table_entry);

uint16_t get_processesIdentifier(InvertedPageTableEntry* inverted_page_table_entry);

void set_validity(InvertedPageTableEntry* entry, bool validity);

void set_processesIdentifier(InvertedPageTableEntry* entry, uint16_t pid);

void set_virtualPageNumber(InvertedPageTableEntry* entry, uint16_t vpn);