#include "bb.h"
#include "string.h"

void bb_attach(bb_t * const bb, uint8_t* data, size_t buffer_size)
{
    assert(bb != NULL);
    assert(data != NULL);
    assert(buffer_size != 0);
    
    bb->data = data;
    bb->data_size = buffer_size;
    bb->index_read = bb->index_write = 0;
}

uint8_t bb_add(bb_t * const bb, uint8_t byte)
{
    if(!bb_is_avail(bb))
        return 0;
    // Положили байт в буффер
    bb->data[bb->index_write++] = byte;
    return 1;
}

// Добавление указанного количества байт
size_t bb_add_bytes(bb_t * const bb, const uint8_t *bytes, size_t size)
{
    assert(bb != NULL);
    assert(bytes != NULL);

    if (size >= bb_availlable(bb))
        return 0;

    memcpy(bb->data + bb->index_write, bytes, size);
    
    return size;
}

size_t bb_availlable(bb_t * const bb)
{
    return bb_get_size(bb) - bb_get_data_lenth(bb);
}

size_t bb_get_size(bb_t * const bb)
{
    assert(bb != NULL);
    return bb->data_size;
}

size_t bb_get_data_lenth(bb_t * const bb)
{
    assert(bb != NULL);
    return bb->index_write;
}

void bb_reject(bb_t * const bb)
{
    assert(bb != NULL);
    bb->index_read = bb->index_write = 0;
}

bool bb_is_avail(bb_t * const bb)
{
    assert(bb != NULL);
    return (bb->data_size - bb->index_write) > 0;
}

size_t bb_unhandled(bb_t * const bb)
{
    assert(bb != NULL);
    return bb->index_write - bb->index_read;
}

uint8_t bb_get_uint8(bb_t * const bb)
{
    assert(bb != NULL);
    return bb->data[bb->index_read++];
}

uint16_t bb_get_uint16(bb_t * const bb)
{
    return bb_get_uint8(bb) << 8 | bb_get_uint8(bb);
}
uint32_t bb_get_uint32(bb_t * const bb)
{
    return  bb_get_uint16(bb) << 16 |
            bb_get_uint16(bb);
}