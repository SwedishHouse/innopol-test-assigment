#include "channel_lvl.h"
#include <pico/stdlib.h>

static bb_t * channel_bb;
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

#define CHANNEL_LVL_START_BYTE 0xFA
#define CHANNEL_LVL_STOP_BYTE 0xFE

static channel_lvl_state_t channel_lvl_state;

void channel_lvl_proccess(void)
{
    while (1)
    {
        while (channel_bb->index_read < channel_bb->index_write)
        {
            switch (channel_lvl_state)
            {
            case CHANNEL_LVL_STATE_NOTHING:
                /* Если что-то доложили в буффер, ищем старт */
                if (channel_bb->index_read < channel_bb->index_write)
                    channel_lvl_state = CHANNEL_LVL_STATE_NEED_START;
                break;
            case CHANNEL_LVL_STATE_NEED_START:
                /* code */
                uint8_t start = bb_get_uint8();
                if (start == CHANNEL_LVL_START_BYTE)
                {
                    channel_lvl_state = CHANNEL_LVL_STATE_NEED_LENGTH;
                }
                break;
            case CHANNEL_LVL_STATE_NEED_LENGTH:
                /* code */
                uint8_t start = bb_get_uint8();
                if (start == CHANNEL_LVL_START_BYTE)
                {
                    channel_lvl_state = CHANNEL_LVL_STATE_NEED_LENGTH;
                }
                

                break;
            
            default:
                assert(0);
                break;
            }
        }
        
    }
    
    
    
}

void channel_lvl_init(bb_t const * byte_buffer)
{
    assert(byte_buffer != NULL);
    channel_bb = byte_buffer;

}