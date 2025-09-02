#include "bb.h"

static bb_t bb = {0};

void bb_attach(uint8_t* buffer, size_t buffer_size)
{
    bb.data = buffer;
    bb.data_size = buffer_size;
    bb.index_read = bb.index_write = 0;
}

void bb_read(void)
{
    
}

void bb_add(uint8_t byte)
{
    // Кольцевой буфер
    bb.data[bb.index_write++%bb.data_size] = byte;
}

size_t bb_get_size()
{
    return bb.data_size;
}

size_t bb_get_data_lenth()
{
    return bb.index_write;
}

uint8_t bb_get_uint8()
{
    uint8_t res = bb.data[bb.index_read];
    bb.index_read++;
    bb.index_read %= bb.data_size;
    return res;
}

uint16_t bb_get_uint16()
{
    return bb_get_uint8() << 8 | bb_get_uint8();
}
uint32_t bb_get_uint32()
{
    return  bb_get_uint16() |
            bb_get_uint16();
}