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

void bb_attach(bb_t * const bb, uint8_t* data, size_t buffer_size);

// Есть ли место для того, что бы положить еще байты
bool bb_is_avail(bb_t * const bb);

// Есть ли место для того, что бы положить еще байты
size_t bb_availlable(bb_t * const bb);

// Число необработанных байт в массиве
size_t bb_unhandled(bb_t * const bb);

// Сбросим указатели чтения и записи в начало
void bb_reject(bb_t * const bb);

// Получаем размер буфера
size_t bb_get_size(bb_t * const bb);
// Количество записанных данных
size_t bb_get_data_lenth(bb_t * const bb);

// Добавление байта
uint8_t bb_add(bb_t * const bb, uint8_t byte);

// Добавление указанного количества байт
size_t bb_add_bytes(bb_t * const bb, const uint8_t *bytes, size_t size);

uint8_t bb_get_uint8(bb_t * const bb);
uint16_t bb_get_uint16(bb_t * const bb);
uint32_t bb_get_uint32(bb_t * const bb);

#endif // __BB_H
