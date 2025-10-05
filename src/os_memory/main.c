#include "../os_memory_API/os_memory_API.h"
#include "stdint.h"
#include "../uint40_t/uint40_t.h"
#include "../process_control_block/process_control_block.h"
#include "../inverted_page_table_entry/inverted_page_table_entry.h"
#include "../frame/frame.h"

char bin_memory_path[100];
ProcessControlBlock* process_control_block_table;
InvertedPageTableEntry* inverted_page_table;
uint8_t* bitmap;
frame* datos;

int main(int argc, char const *argv[]) {
  process_control_block_table = malloc(sizeof(ProcessControlBlock) * 32);
  inverted_page_table = malloc(sizeof(InvertedPageTableEntry) * 65536);
  bitmap = malloc(sizeof(uint8_t) * (65536>>3));
  datos = malloc(sizeof(frame) * (1 << 16));
  
  // montar la memoria

  // mount_memory((char *)argv[1]);

  
  return 0;
}