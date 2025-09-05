#include "channel_lvl.h"
#include <pico/stdlib.h>

// �������� �����, ��������� � ���������������� ������ 
static bb_t * channel_bb_ser;
// �������� �����, ��������� � �������� ���������
static bb_t * channel_bb_reply;

// ������������ ���������
typedef enum
{
  CHANNEL_LVL_STATE_NOTHING,
  CHANNEL_LVL_STATE_NEED_START,
  CHANNEL_LVL_STATE_NEED_MORE_DATA,
  CHANNEL_LVL_STATE_NEED_LENGTH,
  CHANNEL_LVL_STATE_NEED_STOP,
  CHANNEL_LVL_STATE_NEED_HEADER_SRC,
  CHANNEL_LVL_STATE_NEED_START_DATA,
  CHANNEL_LVL_STATE_NEED_MESSAGE_CRC

} channel_lvl_state_t;

// ��������� ���� ������
#define CHANNEL_LVL_START_BYTE 0xFA
// ��������� ����� ������ ������ ������
#define CHANNEL_LVL_START_DATA 0xFB
// ����� ������
#define CHANNEL_LVL_STOP_BYTE 0xFE

// ���������� ���� ��������� ������
#define CHANNEL_LVL_LENGT_SERVICE_BYTES 7

// ��������� ����������� ���������� ������
static channel_lvl_state_t channel_lvl_state;
// ����������� ����� ������
static uint16_t packet_size = 0;

// ������� ����, ��� ����������� ����� ���������� � uint32_t
static uint32_t crc = 0;

// ���������� ��� ��������� � ������
static void channel_lvl_reject(bb_t * const bb)
{
    bb_reject(bb);
    packet_size = 0;
    crc = 0;
    channel_lvl_state = CHANNEL_LVL_STATE_NOTHING;
}

void channel_lvl_proccess(void)
{

    while (1)
    {
        while (bb_unhandled(channel_bb_ser) != 0)
        {
            switch (channel_lvl_state)
            {
            case CHANNEL_LVL_STATE_NOTHING:
                // ���� ���-�� �������� � �����, ���� ����� 
                channel_lvl_state = CHANNEL_LVL_STATE_NEED_START;
                break;
            case CHANNEL_LVL_STATE_NEED_START:

                uint8_t start = bb_get_uint8(channel_bb_ser);
                if (start != CHANNEL_LVL_START_BYTE)
                    break;

                crc += start;
                channel_lvl_state = CHANNEL_LVL_STATE_NEED_LENGTH;
                    
                break;
            case CHANNEL_LVL_STATE_NEED_LENGTH:
                // ���� �� ������ ���� ���� �� �����
                if (bb_unhandled(channel_bb_ser) < 2)
                    break;
                
                packet_size = bb_get_uint16(channel_bb_ser);
                // ���������� ������������ �������. �������, ��� ��� ������� ������ ���� �� ������ ������ �����
                if (packet_size < CHANNEL_LVL_LENGT_SERVICE_BYTES + 1)
                {
                    channel_lvl_reject(channel_bb_ser);
                    break;
                }
                crc += packet_size;
                channel_lvl_state = CHANNEL_LVL_STATE_NEED_HEADER_SRC;
                break;
            
            case CHANNEL_LVL_STATE_NEED_HEADER_SRC:
                
                uint8_t crc_header = bb_get_uint8(channel_bb_ser);
                
                if (crc_header != crc)
                {
                    channel_lvl_reject(channel_bb_ser);
                    break;
                }
                
                crc += crc_header;
                channel_lvl_state = CHANNEL_LVL_STATE_NEED_START_DATA;
                
                break;
            case CHANNEL_LVL_STATE_NEED_START_DATA:
                uint8_t data_start = bb_get_uint8(channel_bb_ser);

                if (data_start != CHANNEL_LVL_START_DATA)
                {
                    // ������������ ������ �������
                    channel_lvl_reject(channel_bb_ser);
                    break;
                }

                crc += data_start;
                channel_lvl_state = CHANNEL_LVL_STATE_NEED_MORE_DATA;
                break;

            case CHANNEL_LVL_STATE_NEED_MORE_DATA:
                // ��������� ���������� ���� 
                if(bb_add(channel_bb_reply, bb_get_uint8(channel_bb_ser)) == 1)
                    break; 

                // ��������� ���������� �����
                channel_lvl_reject(channel_bb_reply);
                
                break;

            default:
                assert(0);
                break;
            }
        }
        
    }
    
    
    
}

void channel_lvl_init(bb_t * const serial_buffer, bb_t * const reply_buffer)
{
    assert(serial_buffer != NULL);
    assert(reply_buffer != NULL);
    bb_attach(channel_bb_ser, serial_buffer->data, serial_buffer->data_size);
    bb_attach(channel_bb_reply, reply_buffer->data, reply_buffer->data_size);

}