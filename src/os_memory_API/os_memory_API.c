#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include <string.h> //para strcmp
#include <stdbool.h> // bool, true, false
#include "os_memory_API.h"

extern char bin_memory_path[100];

// funciones generales
void mount_memory(char* memory_path)
{
    strcpy(bin_memory_path, memory_path);
}


// // funciones procesos

// // funciones archivos