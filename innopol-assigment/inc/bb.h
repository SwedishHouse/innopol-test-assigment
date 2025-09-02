#ifndef __BB_H
#define __BB_H
#include <pico/stdlib.h>


// Для экономии памяти используем один буффер, который объявляется в другом месте программы
typedef struct 
{
    /* data */
    uint8_t *data;
    // Размер учатска памяти
    size_t data_size;
    // Индекс добавления данных
    size_t index_write;
    // Индекс чтения данных
    size_t index_read;
} bb_t;

void bb_attach(uint8_t* buffer, size_t buffer_size);

size_t bb_get_size();
size_t bb_get_data_lenth();

void bb_read(void);

void bb_add(uint8_t byte);

uint8_t bb_get_uint8(void);
uint16_t bb_get_uint16(void);
uint32_t bb_get_uint32(void);

#endif // __BB_H
