#ifndef __BB_H
#define __BB_H
#include <pico/stdlib.h>


// ��� �������� ������ ���������� ���� ������, ������� ����������� � ������ ����� ���������
typedef struct 
{
    /* data */
    uint8_t *data;
    // ������ ������� ������
    size_t data_size;
    // ������ ���������� ������
    size_t index_write;
    // ������ ������ ������
    size_t index_read;
} bb_t;

void bb_attach(bb_t * const bb, uint8_t* data, size_t buffer_size);

// ���� �� ����� ��� ����, ��� �� �������� ��� �����
bool bb_is_avail(bb_t * const bb);

// ���� �� ����� ��� ����, ��� �� �������� ��� �����
size_t bb_availlable(bb_t * const bb);

// ����� �������������� ���� � �������
size_t bb_unhandled(bb_t * const bb);

// ������� ��������� ������ � ������ � ������
void bb_reject(bb_t * const bb);

// �������� ������ ������
size_t bb_get_size(bb_t * const bb);
// ���������� ���������� ������
size_t bb_get_data_lenth(bb_t * const bb);

// ���������� �����
uint8_t bb_add(bb_t * const bb, uint8_t byte);

// ���������� ���������� ���������� ����
size_t bb_add_bytes(bb_t * const bb, const uint8_t *bytes, size_t size);

uint8_t bb_get_uint8(bb_t * const bb);
uint16_t bb_get_uint16(bb_t * const bb);
uint32_t bb_get_uint32(bb_t * const bb);

#endif // __BB_H
