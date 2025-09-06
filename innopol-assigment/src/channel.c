#include "channel.h"
#include <pico/stdlib.h>

// Коллбэк успешного приема данных
channel_packet_cb channel_packet_rec;


// Байтовый буфер, связанный с последовательным портом 
static bb_t * channel_bb_ser;
// Байтовый буфер, связанный с полезной нагрузкой
static bb_t * channel_bb_reply;

// Перечисление состояний
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

// Стартовый байт пакета
#define CHANNEL_START_BYTE 0xFA
// Стартовый буфер данных внутри пакета
#define CHANNEL_START_DATA 0xFB
// Конец пакета
#define CHANNEL_STOP_BYTE 0xFE

// Количество байт служебных данных
#define CHANNEL_LENGT_SERVICE_BYTES 7

// Состояние обработчика канального уровня
static channel_state_t channel_state;
// Вычисленная длина пакета
static uint16_t channel_packet_size = 0;

// Считаем пока, что контрольная сумма помещается в uint32_t
static uint32_t channel_crc = 0;

// Сбрасываем все состояния в начало
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
                // Ничего не делаем
                
                break;
            case CHANNEL_STATE_NEED_START:

                uint8_t start = bb_get_uint8(channel_bb_ser);
                if (start != CHANNEL_START_BYTE)
                    break;
                // Инкремент суммы 
                channel_crc += start;
                // Переведем состояние
                channel_state = CHANNEL_STATE_NEED_LENGTH;
                    
                break;
            case CHANNEL_STATE_NEED_LENGTH:
                // Ждем не меньше двух байт на длину
                if (bb_unhandled(channel_bb_ser) < 2)
                    break;
                
                channel_packet_size = bb_get_uint16(channel_bb_ser);

                // Меняем порядок байт
                channel_packet_size = (channel_packet_size << 8) | (uint16_t)(channel_packet_size >> 8);
                // Обработаем некорректные размеры. Считаем, что имя функции должно быть не меньше одного байта
                if (channel_packet_size < CHANNEL_LENGT_SERVICE_BYTES + 1)
                {
                    channel_reject();
                    break;
                }
                // Инкремент суммы 
                channel_crc += channel_packet_size;
                // уберем уже обработанные данные
                channel_packet_size -= 3;
                // Переведем состояние
                channel_state = CHANNEL_STATE_NEED_HEADER_SRC;
                break;
            
            case CHANNEL_STATE_NEED_HEADER_SRC:
                
                uint8_t crc_header = bb_get_uint8(channel_bb_ser);
                
                if (crc_header != channel_crc)
                {
                    channel_reject();
                    break;
                }

                // Инкремент суммы 
                channel_crc += crc_header;
                // уберем уже обработанные данные
                channel_packet_size -= 1;
                // Переведем состояние
                channel_state = CHANNEL_STATE_NEED_START_DATA;
                
                break;
            case CHANNEL_STATE_NEED_START_DATA:
                uint8_t data_start = bb_get_uint8(channel_bb_ser);

                if (data_start != CHANNEL_START_DATA)
                {
                    // Неправильный формат посылки
                    channel_reject();
                    break;
                }
                // Инкремент суммы 
                channel_crc += data_start;
                // уберем уже обработанные данные
                channel_packet_size -= 1;
                // Переведем состояние
                channel_state = CHANNEL_STATE_NEED_MORE_DATA;
                break;

            case CHANNEL_STATE_NEED_MORE_DATA:
                // Докладываем данные в буффер транспортного уровня
                // Обработка данных до тех пор, пока не останется сумма всего пакета и стоповый бит 
                if(channel_packet_size == 2)
                {
                    channel_state = CHANNEL_STATE_NEED_MESSAGE_CRC;
                    break;
                }
                else if (channel_packet_size < 2)
                {
                    // Какая-то ошибка
                    channel_reject();
                    bb_reject(channel_bb_reply);
                }
                
                
                // Копирование в буфер транспортного уровня
                const uint8_t data_byte = bb_get_uint8(channel_bb_ser);
                bb_add(channel_bb_reply, data_byte);
                // Инкремент суммы 
                channel_crc += data_byte;
                // уберем уже обработанные данные
                channel_packet_size -= 1;
                
                break;

                case CHANNEL_STATE_NEED_MESSAGE_CRC:
                    const uint8_t data_byte = bb_get_uint8(channel_bb_ser);
                    // Инкремент суммы 
                    if(channel_crc != data_byte)
                    {
                        // Контролньая сумма пакета не сошлась
                        // В целях упрощения жизни стоповый байт не включен в эту сумму
                        channel_reject();
                        bb_reject(channel_bb_reply);
                    }
                        
                    // уберем уже обработанные данные
                    channel_packet_size -= 1;
                    channel_state = CHANNEL_STATE_NEED_STOP;
                break;

                case CHANNEL_STATE_NEED_STOP:
                    const uint8_t data_byte = bb_get_uint8(channel_bb_ser);
                    // Инкремент суммы 
                    if(data_byte != CHANNEL_STOP_BYTE)
                    {
                        // Контролньая сумма пакета не сошлась
                        // В целях упрощения жизни стоповый байт не включен в эту сумму
                        channel_reject();
                        bb_reject(channel_bb_reply);
                    }
                        
                    // Уберем уже обработанные данные
                    channel_packet_size -= 1;
                    channel_state = CHANNEL_STATE_NEED_STOP;
                    
                    // Пока считаю, что между запросами достаточно времени, потому просто сброшу буфер
                    channel_reject();
                    // Отправка сообщения на транспортный уровень
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
    // Проинициализируем буферы
    bb_attach(channel_bb_ser, serial_buffer->data, serial_buffer->data_size);
    bb_attach(channel_bb_reply, reply_buffer->data, reply_buffer->data_size);
    // Сбросим буферы
    channel_reject();
    bb_reject(channel_bb_reply);

    // Устанавливаем коллбэк успешного приема данных
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

    // посчитаем контрольную сумму данных
    uint8_t crc = 0;
    size_t index = 0;

    // Добавим стартовый байт
    out_buffer[index++] = CHANNEL_START_BYTE;
    crc += CHANNEL_START_BYTE;

    // Подсчитаем длину пакета
    {
        const uint16_t packet_length = CHANNEL_LENGT_SERVICE_BYTES + data_size;

        // Добавим младший байт размера
        out_buffer[index++] = (uint8_t)packet_length;
        crc += (uint8_t)packet_length;
        // Добавим старший байт размера
        out_buffer[index++] = (uint8_t)(packet_length >> 8);
        crc += (uint8_t)(packet_length >> 8);
    }
    // Контрольная суммв заголовка
    out_buffer[index++] = crc;

    // Стартовый байт данных
    out_buffer[index++] = CHANNEL_START_DATA;
    crc += CHANNEL_START_DATA;

    // Копируем сообщение
    for (size_t i = 0; i < data_size; i++)
    {
        out_buffer[index++] = data[i];
        crc += data[i];
    }

    // Добавляю контрольную сумму
    out_buffer[index++] = crc;

    // Добавлю стоповый байт
    out_buffer[index++] = CHANNEL_STOP_BYTE;

    // Должны совпасть
    assert(index == CHANNEL_LENGT_SERVICE_BYTES + data_size);
}