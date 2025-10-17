#include "osm_File.h"

int get_virtual_page_number_from_virtual_adress(int32_t virtual_adress)
{
    return (virtual_adress >> 15) & 0b00000000000000000000111111111111;
}