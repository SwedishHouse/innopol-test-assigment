#ifndef __SERIAL_RPC_H
#define __SERIAL_H
#include <pico/stdlib.h>

void serial_init(uint8_t * data, size_t data_size);

void serial_write(uint8_t *data, size_t size);

size_t serial_read(uint8_t *dest);

#endif  // __SERIAL_H