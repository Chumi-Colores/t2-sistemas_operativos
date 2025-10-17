#include "../os_memory_API/os_memory_API.h"
#include "stdint.h"
#include "../process_control_block_table/process_control_block_table.h"
#include "../inverted_page_table/inverted_page_table.h"
#include "../frame_bitmap/frame_bitmap.h"
#include "../data/data.h"

char bin_memory_path[100];
ProcessControlBlockTable process_control_block_table;
InvertedPageTable inverted_page_table;
FrameBitmap frame_bitmap;
Data data;

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Uso: %s <ruta_archivo_memoria>\n", argv[0]);
    return EXIT_FAILURE;
  }
  initialize_ProcessControlBlockTable(&process_control_block_table, 32);
  initialize_InvertedPageTable(&inverted_page_table, 65536);
  initialize_FrameBitmap(&frame_bitmap, 65536>>3);
  initialize_Data(&data, 1 << 16);
  

  mount_memory((char *)argv[1]);

  // commandos test
  list_processes();

  printf("Slots libres para procesos: %d\n", processes_slots());

  list_files(198);

  frame_bitmap_status();

  // Liberar memoria
  free(process_control_block_table.entries);
  free(inverted_page_table.entries);
  free(frame_bitmap.bytes);
  free(data.frames);
}