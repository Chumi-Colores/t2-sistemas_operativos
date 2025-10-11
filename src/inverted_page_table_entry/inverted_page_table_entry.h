#pragma once
#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"

typedef struct {
    unsigned char data[3]; // NNNNNNNN NNNNNPPP PPPPPPPV
    // Donde los N son parte de VPN, los P son parte del pid y la V es el bit de validez
} InvertedPageTableEntry;

bool get_validity(InvertedPageTableEntry inverted_page_table_entry);

uint16_t get_virtualPageNumber(InvertedPageTableEntry inverted_page_table_entry);

uint16_t get_processesIdentifier(InvertedPageTableEntry inverted_page_table_entry);

void set_validity(InvertedPageTableEntry* entry, bool validity);

void set_processesIdentifier(InvertedPageTableEntry* entry, uint16_t pid);

void set_virtualPageNumber(InvertedPageTableEntry* entry, uint16_t vpn);