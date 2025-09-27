#include "transport.h"

// Байтовый буфер приема сообщений
bb_t *  transport_input;

static bool transport_is_new_functs;

// Указатель на коллбэки
static struct
{
    transport_exec_cb_t *cb;
    size_t cb_size;
    char **names;
    int * setting_arr;
    size_t index;
}transport_exec_s;


void transport_new_message_cb(void)
{
    transport_is_new_functs = true;
}

void transport_init(bb_t * const reply_buffer, const transport_exec_cb_t *functs, 
    size_t size, char *functs_names[], int * setting_arr)
{
    assert(reply_buffer != NULL);
    assert(functs != NULL);
    assert(functs_names != NULL);
    assert(setting_arr != NULL);
    assert(size != 0);
 
    // Проинициализируем буфер
    bb_attach(transport_input, reply_buffer->data, reply_buffer->data_size);
    // Сбросим буфер
    bb_reject(transport_input);

    transport_is_new_functs = false;
    transport_exec_s.cb = functs;
    transport_exec_s.cb_size = size;
    transport_exec_s.names = functs_names;
    transport_exec_s.setting_arr = setting_arr;
    transport_exec_s.index = 0;
}

// Перечисление состояний
enum transport_state
{
  TRANSPORT_STATE_NOTHING,
  TRANSPORT_STATE_NEED_TO_HANDLE,
  TRANSPORT_STATE_NEED_MORE_DATA,
  TRANSPORT_STATE_NEED_LENGTH,
  TRANSPORT_STATE_NEED_STOP,
  TRANSPORT_STATE_NEED_HEADER_SRC,
  TRANSPORT_STATE_NEED_START_DATA,
  TRANSPORT_STATE_NEED_MESSAGE_CRC
};

enum transport_message_type
{
  TRANSPORT_MESSAGE_TYPE_REQUEST    = 0x0B,
  TRANSPORT_MESSAGE_TYPE_STREAM     = 0x0C,
  TRANSPORT_MESSAGE_TYPE_ANSWER     = 0x16,
  TRANSPORT_MESSAGE_TYPE_ERROR      = 0x21
};


void transport_process_input(void)
{
    static enum transport_state state = TRANSPORT_STATE_NOTHING;
    while (1)
    {
        switch (state)
        {
        case TRANSPORT_STATE_NOTHING:
            /* code */
            if (transport_is_new_functs)
            {
                state = TRANSPORT_STATE_NEED_TO_HANDLE;
            }
            
            break;
        case TRANSPORT_STATE_NEED_TO_HANDLE:
            const enum transport_message_type msg_type = bb_get_uint8(transport_input);
            
            

            break;
        
        default:
            assert(false);
            break;
        }
        
    }
    

}

