#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"

typedef struct {
    unsigned char data[3];
    // Estructura de bits (de LSB a MSB):
    // data[0]: PPPPPPPV   (7 bits de PID + 1 bit validez)
    // data[1]: NNNNNPPP   (5 bits de VPN + 3 bits de PID)
    // data[2]: NNNNNNNN   (8 bits de VPN)
    //
    // V  -> bit de validez (bit 0 de data[0])
    // PID -> bits 1–10
    // VPN -> bits 11–23
} InvertedPageTableEntry;

bool get_validity(InvertedPageTableEntry* inverted_page_table_entry);

uint16_t get_virtualPageNumber(InvertedPageTableEntry* inverted_page_table_entry);

uint16_t get_processesIdentifier(InvertedPageTableEntry* inverted_page_table_entry);

void set_validity(InvertedPageTableEntry* entry, bool validity);

void set_processesIdentifier(InvertedPageTableEntry* entry, uint16_t pid);

void set_virtualPageNumber(InvertedPageTableEntry* entry, uint16_t vpn);