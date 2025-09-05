#ifndef __CHANNEL_LVL_H
#define __CHANNEL_LVL_H

#include "bb.h"

// Библиотека работы канального уровня

void channel_lvl_init(bb_t  * const serial_buffer, bb_t * const reply_buffer);

void channel_lvl_proccess(void);

#endif // __CHANNEL_LVL_H