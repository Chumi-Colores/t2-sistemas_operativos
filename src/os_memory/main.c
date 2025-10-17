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
const int FILES_PER_PROCESS = 10;

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

  int play = 1;
  int there_is_an_open_file = 0;
  int free_slots = 0;
  int process_id = -1;
  char process_name[14];
  int pid_from_started_file;
  int terminated_count = 0;
  int slots;
  char file_name[14];
  char mode;
  char dest[14];
  char source[14];
  osmFile* opened_file = NULL;
  while (play) {
    // get console input
    printf("-----------------------------------------------------------\n");
    printf("Ingrese un comando: \n");
    printf("0- Salir\n");
    printf("1- list_processes\n");
    printf("2- processes_slots\n");
    printf("3- list_files <process_id>\n");
    printf("4- frame_bitmap\n");
    printf("5- start_process <process_name>\n");
    printf("6- finish_process <process_id>\n");
    printf("7- clear_all_processes\n");
    printf("8- file_table_slots <process_id>\n");
    printf("9- open_file <process_id> <file_name> <mode>\n");
    printf("10- delete_file <process_id> <file_name>\n");
    if (there_is_an_open_file) {
      printf("11- read_file <dest>\n");
      printf("12- write_file <source>\n");
      printf("13- close_file\n");
    }
    else {
      printf("El resto de funciones de archivos aparecen tras abrir un archivo\n");
    }
    // get input
    int input;
    scanf("%d", &input);
    printf("-----------------------------------------------------------\n");
    switch (input) {
      case 0:
        play = 0;
        break;
      case 1:
        list_processes();
        break;
      case 2:
        free_slots = processes_slots();
        printf("Slots libres para procesos: %d\n", free_slots);
        break;
      case 3: {
        printf("Ingrese el ID del proceso: ");
        scanf("%d", &process_id);
        list_files(process_id);
        break;
      }
      case 4:
        frame_bitmap_status();
        break;
      case 5: {
        printf("Ingrese el ID del proceso: ");
        scanf("%d", &process_id);
        printf("Ingrese el nombre del proceso: (máx 14 caracteres)");
        scanf("%s", process_name);
        pid_from_started_file = start_process(process_id, process_name);
        if (pid_from_started_file != -1) {
          printf("Proceso iniciado con PID: %d\n", pid_from_started_file);
        } else {
          printf("No se pudo iniciar el proceso.\n");
        }
        break;
      }
      case 6: {
        printf("Ingrese el ID del proceso a finalizar: ");
        scanf("%d", &process_id);
        if (finish_process(process_id) != -1) {
          printf("Proceso %d finalizado.\n", process_id);
        } else {
          printf("No se pudo finalizar el proceso %d.\n", process_id);
        }
        break;
      }
      case 7: {
        terminated_count = clear_all_processes();
        printf("Procesos terminados: %d\n", terminated_count);
        break;
      }
      case 8: {
        printf("Ingrese el ID del proceso: ");
        scanf("%d", &process_id);
        slots = file_table_slots(process_id);
        if (slots != -1) {
          printf("Slots libres en la tabla de archivos del proceso %d: %d\n", process_id, slots);
        } else {
          printf("No se pudo obtener los slots para el proceso %d.\n", process_id);
        }
        break;
      }
      case 9: {
        printf("Ingrese el ID del proceso que desea abrir el archivo: ");
        scanf("%d", &process_id);
        printf("Ingrese el nombre del archivo: (máx 14 caracteres) ");
        scanf("%s", file_name);
        printf("Ingrese el modo de apertura (r/w): ");
        scanf("%c", &mode);
        opened_file = open_file(process_id, file_name, mode);
        if (opened_file != NULL) {
          there_is_an_open_file = 1;
          printf("Archivo abierto exitosamente.\n");
        } else {
          printf("No se pudo abrir el archivo.\n");
        }
        break;
      }
      case 10: {
        printf("Ingrese el ID del proceso que desea eliminar el archivo: ");
        scanf("%d", &process_id);
        printf("Ingrese el nombre del archivo a eliminar: (máx 14 caracteres) ");
        list_files(process_id);
        scanf("%s", file_name);
        delete_file(process_id, file_name);
        printf("Archivo eliminado si existía.\n");
        break;
      }
      case 11:
        if (there_is_an_open_file) {
          printf("Ingrese el destino del archivo a leer: (máx 14 caracteres)");
          scanf("%s", dest);
          read_file(opened_file, dest);
        } else {
          printf("Ingrese un comando válido.\n");
        }
        break;
      case 12:
        if (there_is_an_open_file) {
          printf("Ingrese la fuente del archivo a escribir: (máx 14 caracteres)");
          scanf("%s", source);
          write_file(opened_file, source);
        } else {
          printf("Ingrese un comando válido.\n");
        }
        break;
      case 13:
        if (there_is_an_open_file) {
          there_is_an_open_file = 0;
          close_file(opened_file);
          opened_file = NULL;
          printf("Archivo cerrado exitosamente.\n");
        } else {
          printf("Ingrese un comando válido.\n");
        }
        break;
      default:
        printf("Comando no reconocido.\n");
        break;
    }
  }


  // Liberar memoria
  free(process_control_block_table.entries);
  free(inverted_page_table.entries);
  free(frame_bitmap.bytes);
  free(data.frames);
}