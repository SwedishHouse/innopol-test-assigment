#ifndef __CHANNEL_H
#define __CHANNEL_H

#include "bb.h"

typedef void (* channel_packet_cb)(void);

// Библиотека работы канального уровня

void channel_init(bb_t * const serial_buffer, bb_t * const reply_buffer, channel_packet_cb cb_recv);

void channel_proccess_input(void);

void channel_process_output(const uint8_t * data, size_t data_size);

void channel_start(void);

void channel_stop(void);

void channel_discard(void);

#endif // __CHANNEL_H