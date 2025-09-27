#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include "bb.h"
#include <pico/stdlib.h>

typedef struct
{
    /* data */
    uint8_t arg_len;
    uint8_t *args;
}transport_funct_arg_t;

typedef void (* transport_exec_cb_t)(transport_funct_arg_t);

// Функция инициализации транспортного уровня принимает байтовый буфер приема сообщений,
//  указатель на обработчики событий, и их размер, и соотствующего размера массив имен функций
void transport_init(bb_t * const reply_buffer, const transport_exec_cb_t *functs, 
    size_t size, char *functs_names[], int * setting_arr);

void transport_new_message_cb(void);

void transport_process_input(void);

void transport_process_output(void);

#endif // __TRANSPORT_H