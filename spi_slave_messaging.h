#ifndef SPI_SLAVE_MESSAGING_H_
#define SPI_SLAVE_MESSAGING_H_

#include <stdint.h>

#define SPI_SLAVE_MESSAGE_INFO_LEN 0
#define SPI_SLAVE_MESSAGE_INFO_TYPE 1

#define SPI_SLAVE_MESSAGE_TYPE_NORMAL 0
#define SPI_SLAVE_MESSAGE_TYPE_DUMMY 1

extern const uint8_t process_window_trigger_event[3];
#define PROCESS_WINDOW_TRG_SIZE (sizeof(process_window_trigger_event))


#define SPI_SLAVE_MESSAGE_PROCESS_TRIGGER_LOC 0
#define SPI_SLAVE_MESSAGE_LEN_LOC PROCESS_WINDOW_TRG_SIZE
#define SPI_SLAVE_MESSAGE_TYPE_LOC (SPI_SLAVE_MESSAGE_LEN_LOC+sizeof(uint16_t))
#define SPI_SLAVE_MESSAGE_PAYLOAD_LOC (SPI_SLAVE_MESSAGE_TYPE_LOC+sizeof(uint8_t))


#define SPI_SLAVE_MESSAGING_BUFSIZE_TO_ADD SPI_SLAVE_MESSAGE_PAYLOAD_LOC

typedef struct spi_slave_message_s spi_slave_message_s;
typedef void (*msg_process_cb)(struct spi_slave_message_s *msg_info);
typedef void (*spi_out_func)(uint8_t val);

typedef struct spi_slave_message_s
{
	uint8_t cur_state;
//	uint8_t process_window_cnt;
//
//	uint8_t len_msg_cnt;
	uint16_t len_msg;
	uint8_t type_msg;
//	uint8_t len_msg_buf[sizeof(uint16_t)];

	uint16_t cur_cnt;
	uint16_t msg_max_size;
	uint8_t *msg_ptr_in;
	uint8_t *msg_ptr_out;

	msg_process_cb process_cb;
	spi_out_func spi_out_cb;

}spi_slave_message_t;

void spi_slave_message_init(spi_slave_message_t *msg_info,
							uint16_t buf_size_in,
							uint8_t *msg_ptr_in,
							uint8_t *msg_ptr_out,
							msg_process_cb cb);

void spi_slave_message_set_out_cb(spi_slave_message_t *msg_info, spi_out_func cb);

void spi_slave_message_process(spi_slave_message_t *msg_info, uint8_t in_char);
uint16_t spi_slave_message_encode_header(uint8_t *msg_out, uint16_t msg_len, uint8_t type);
uint16_t spi_slave_message_encode(uint8_t *msg_out, uint16_t msg_len, uint8_t *msg_in, uint8_t msg_fill_value);
int16_t spi_slave_message_get_info(spi_slave_message_t *msg_info, uint16_t what);

#ifdef __linux__
void spi_slave_message_swap_endianess(uint8_t *msg);
#endif

#endif /* SPI_SLAVE_MESSAGING_H_ */
