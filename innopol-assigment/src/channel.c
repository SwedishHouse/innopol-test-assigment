#include "channel.h"
#include <pico/stdlib.h>

// ������� ��������� ������ ������
channel_packet_cb channel_packet_rec;


// �������� �����, ��������� � ���������������� ������ 
static bb_t * channel_bb_ser;
// �������� �����, ��������� � �������� ���������
static bb_t * channel_bb_reply;

// ������������ ���������
typedef enum
{
  CHANNEL_STATE_NOTHING,
  CHANNEL_STATE_NEED_START,
  CHANNEL_STATE_NEED_MORE_DATA,
  CHANNEL_STATE_NEED_LENGTH,
  CHANNEL_STATE_NEED_STOP,
  CHANNEL_STATE_NEED_HEADER_SRC,
  CHANNEL_STATE_NEED_START_DATA,
  CHANNEL_STATE_NEED_MESSAGE_CRC

} channel_state_t;

// ��������� ���� ������
#define CHANNEL_START_BYTE 0xFA
// ��������� ����� ������ ������ ������
#define CHANNEL_START_DATA 0xFB
// ����� ������
#define CHANNEL_STOP_BYTE 0xFE

// ���������� ���� ��������� ������
#define CHANNEL_LENGT_SERVICE_BYTES 7

// ��������� ����������� ���������� ������
static channel_state_t channel_state;
// ����������� ����� ������
static uint16_t channel_packet_size = 0;

// ������� ����, ��� ����������� ����� ���������� � uint32_t
static uint32_t channel_crc = 0;

// ���������� ��� ��������� � ������
static void channel_reject(void)
{
    bb_reject(channel_bb_ser);
    channel_packet_size = 0;
    channel_crc = 0;
    channel_state = CHANNEL_STATE_NEED_START;
}

void channel_proccess_input(void)
{
    while (1)
    {
        while (bb_unhandled(channel_bb_ser) != 0)
        {
            switch (channel_state)
            {
            case CHANNEL_STATE_NOTHING:
                // ������ �� ������
                
                break;
            case CHANNEL_STATE_NEED_START:

                uint8_t start = bb_get_uint8(channel_bb_ser);
                if (start != CHANNEL_START_BYTE)
                    break;
                // ��������� ����� 
                channel_crc += start;
                // ��������� ���������
                channel_state = CHANNEL_STATE_NEED_LENGTH;
                    
                break;
            case CHANNEL_STATE_NEED_LENGTH:
                // ���� �� ������ ���� ���� �� �����
                if (bb_unhandled(channel_bb_ser) < 2)
                    break;
                
                channel_packet_size = bb_get_uint16(channel_bb_ser);

                // ������ ������� ����
                channel_packet_size = (channel_packet_size << 8) | (uint16_t)(channel_packet_size >> 8);
                // ���������� ������������ �������. �������, ��� ��� ������� ������ ���� �� ������ ������ �����
                if (channel_packet_size < CHANNEL_LENGT_SERVICE_BYTES + 1)
                {
                    channel_reject();
                    break;
                }
                // ��������� ����� 
                channel_crc += channel_packet_size;
                // ������ ��� ������������ ������
                channel_packet_size -= 3;
                // ��������� ���������
                channel_state = CHANNEL_STATE_NEED_HEADER_SRC;
                break;
            
            case CHANNEL_STATE_NEED_HEADER_SRC:
                
                uint8_t crc_header = bb_get_uint8(channel_bb_ser);
                
                if (crc_header != channel_crc)
                {
                    channel_reject();
                    break;
                }

                // ��������� ����� 
                channel_crc += crc_header;
                // ������ ��� ������������ ������
                channel_packet_size -= 1;
                // ��������� ���������
                channel_state = CHANNEL_STATE_NEED_START_DATA;
                
                break;
            case CHANNEL_STATE_NEED_START_DATA:
                uint8_t data_start = bb_get_uint8(channel_bb_ser);

                if (data_start != CHANNEL_START_DATA)
                {
                    // ������������ ������ �������
                    channel_reject();
                    break;
                }
                // ��������� ����� 
                channel_crc += data_start;
                // ������ ��� ������������ ������
                channel_packet_size -= 1;
                // ��������� ���������
                channel_state = CHANNEL_STATE_NEED_MORE_DATA;
                break;

            case CHANNEL_STATE_NEED_MORE_DATA:
                // ����������� ������ � ������ ������������� ������
                // ��������� ������ �� ��� ���, ���� �� ��������� ����� ����� ������ � �������� ��� 
                if(channel_packet_size == 2)
                {
                    channel_state = CHANNEL_STATE_NEED_MESSAGE_CRC;
                    break;
                }
                else if (channel_packet_size < 2)
                {
                    // �����-�� ������
                    channel_reject();
                    bb_reject(channel_bb_reply);
                }
                
                
                // ����������� � ����� ������������� ������
                const uint8_t data_byte = bb_get_uint8(channel_bb_ser);
                bb_add(channel_bb_reply, data_byte);
                // ��������� ����� 
                channel_crc += data_byte;
                // ������ ��� ������������ ������
                channel_packet_size -= 1;
                
                break;

                case CHANNEL_STATE_NEED_MESSAGE_CRC:
                    const uint8_t data_byte = bb_get_uint8(channel_bb_ser);
                    // ��������� ����� 
                    if(channel_crc != data_byte)
                    {
                        // ����������� ����� ������ �� �������
                        // � ����� ��������� ����� �������� ���� �� ������� � ��� �����
                        channel_reject();
                        bb_reject(channel_bb_reply);
                    }
                        
                    // ������ ��� ������������ ������
                    channel_packet_size -= 1;
                    channel_state = CHANNEL_STATE_NEED_STOP;
                break;

                case CHANNEL_STATE_NEED_STOP:
                    const uint8_t data_byte = bb_get_uint8(channel_bb_ser);
                    // ��������� ����� 
                    if(data_byte != CHANNEL_STOP_BYTE)
                    {
                        // ����������� ����� ������ �� �������
                        // � ����� ��������� ����� �������� ���� �� ������� � ��� �����
                        channel_reject();
                        bb_reject(channel_bb_reply);
                    }
                        
                    // ������ ��� ������������ ������
                    channel_packet_size -= 1;
                    channel_state = CHANNEL_STATE_NEED_STOP;
                    
                    // ���� ������, ��� ����� ��������� ���������� �������, ������ ������ ������ �����
                    channel_reject();
                    // �������� ��������� �� ������������ �������
                    channel_packet_rec();

                break;


            default:
                assert(0);
                break;
            }
        }
    }
}

void channel_init(bb_t * const serial_buffer, bb_t * const reply_buffer, channel_packet_cb cb_recv)
{
    assert(serial_buffer != NULL);
    assert(reply_buffer != NULL);
    // ����������������� ������
    bb_attach(channel_bb_ser, serial_buffer->data, serial_buffer->data_size);
    bb_attach(channel_bb_reply, reply_buffer->data, reply_buffer->data_size);
    // ������� ������
    channel_reject();
    bb_reject(channel_bb_reply);

    // ������������� ������� ��������� ������ ������
    channel_packet_rec = cb_recv;
}

void channel_start(void)
{
    channel_state = CHANNEL_STATE_NEED_START;
}

void channel_stop(void)
{
    channel_state = CHANNEL_STATE_NOTHING;
}

void channel_discard(void)
{
    channel_reject();
}

#define CHANNEL_OUT_BUFFER_SIZE 1024

void channel_proccess_output(const uint8_t * data, size_t data_size)
{
    static uint8_t out_buffer[CHANNEL_OUT_BUFFER_SIZE];

    assert(data != NULL);
    assert(data_size != 0);
    assert(data_size + CHANNEL_LENGT_SERVICE_BYTES < CHANNEL_OUT_BUFFER_SIZE);

    // ��������� ����������� ����� ������
    uint8_t crc = 0;
    size_t index = 0;

    // ������� ��������� ����
    out_buffer[index++] = CHANNEL_START_BYTE;
    crc += CHANNEL_START_BYTE;

    // ���������� ����� ������
    {
        const uint16_t packet_length = CHANNEL_LENGT_SERVICE_BYTES + data_size;

        // ������� ������� ���� �������
        out_buffer[index++] = (uint8_t)packet_length;
        crc += (uint8_t)packet_length;
        // ������� ������� ���� �������
        out_buffer[index++] = (uint8_t)(packet_length >> 8);
        crc += (uint8_t)(packet_length >> 8);
    }
    // ����������� ����� ���������
    out_buffer[index++] = crc;

    // ��������� ���� ������
    out_buffer[index++] = CHANNEL_START_DATA;
    crc += CHANNEL_START_DATA;

    // �������� ���������
    for (size_t i = 0; i < data_size; i++)
    {
        out_buffer[index++] = data[i];
        crc += data[i];
    }

    // �������� ����������� �����
    out_buffer[index++] = crc;

    // ������� �������� ����
    out_buffer[index++] = CHANNEL_STOP_BYTE;

    // ������ ��������
    assert(index == CHANNEL_LENGT_SERVICE_BYTES + data_size);
}