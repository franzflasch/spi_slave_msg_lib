#include <spi_slave_messaging.h>
#include <string.h>
#include <stdio.h>

#define SPI_SLAVE_MSG_STATE_BEGIN 0
#define SPI_SLAVE_MSG_STATE_GET_LEN 1
#define SPI_SLAVE_MSG_STATE_GET_TYPE 2
#define SPI_SLAVE_MSG_STATE_PROCESS 3

const uint8_t process_window_trigger_event[] = "9h4";

void spi_slave_message_init(spi_slave_message_t *msg_info,
							uint16_t buf_size_in,
							uint8_t *msg_ptr_in,
							uint8_t *msg_ptr_out,
							msg_process_cb cb)
{
	msg_info->cur_state = SPI_SLAVE_MSG_STATE_BEGIN;

//	msg_info->process_window_cnt = 0;
//
//	msg_info->len_msg_cnt = 0;
//	msg_info->len_msg = 0;
//	memset(msg_info->len_msg_buf, 0, sizeof(uint16_t));

	msg_info->cur_cnt = 0;
	msg_info->msg_max_size = buf_size_in;
	msg_info->msg_ptr_in = msg_ptr_in;
	msg_info->msg_ptr_out = msg_ptr_out;

	msg_info->process_cb = cb;
}

void spi_slave_message_set_out_cb(spi_slave_message_t *msg_info, spi_out_func cb)
{
	msg_info->spi_out_cb = cb;
}

static void reset_values(spi_slave_message_t *msg_info)
{
	msg_info->cur_state = SPI_SLAVE_MSG_STATE_BEGIN;
	msg_info->cur_cnt = 0;
	msg_info->len_msg = 0;
}

void spi_slave_message_process(spi_slave_message_t *msg_info, uint8_t in_char)
{
	msg_info->msg_ptr_in[msg_info->cur_cnt] = in_char;

	if(msg_info->cur_state == SPI_SLAVE_MSG_STATE_BEGIN)
	{
		/* Check if input matches the process_trigger_window */
		if(in_char != process_window_trigger_event[msg_info->cur_cnt])
		{
			reset_values(msg_info);
		}

		if(msg_info->cur_cnt == (PROCESS_WINDOW_TRG_SIZE-1))
		{
			msg_info->cur_state = SPI_SLAVE_MSG_STATE_GET_LEN;
		}
	}
	else if(msg_info->cur_state == SPI_SLAVE_MSG_STATE_GET_LEN)
	{
		//msg_info->len_msg_buf[msg_info->len_msg_cnt++] = in_char;
		//printf("S%d\r\n", in_char);
		if(msg_info->cur_cnt == (SPI_SLAVE_MESSAGE_TYPE_LOC-1))
		{
			memcpy(&msg_info->len_msg, &msg_info->msg_ptr_in[SPI_SLAVE_MESSAGE_LEN_LOC], sizeof(uint16_t));

			//printf("l:%d\r\n", msg_info->len_msg);

			if(msg_info->len_msg > 0)
			{
				/* check if size is appropriate */
				if(msg_info->len_msg > msg_info->msg_max_size)
				{
					reset_values(msg_info);
				}
				else
				{
					msg_info->cur_state = SPI_SLAVE_MSG_STATE_GET_TYPE;
				}
			}
			else
			{
				msg_info->cur_state = SPI_SLAVE_MSG_STATE_BEGIN;
			}
		}
	}
	else if(msg_info->cur_state == SPI_SLAVE_MSG_STATE_GET_TYPE)
	{
		//printf("S%d\r\n", in_char);

		//printf("R%d", SPI_SLAVE_MESSAGE_TYPE_LOC);
		if(msg_info->cur_cnt == SPI_SLAVE_MESSAGE_TYPE_LOC)
		{
			msg_info->type_msg = msg_info->msg_ptr_in[SPI_SLAVE_MESSAGE_TYPE_LOC];
			msg_info->cur_state = SPI_SLAVE_MSG_STATE_PROCESS;
			//printf("l:%d\r\n", msg_info->type_msg);
		}
		else
		{
			reset_values(msg_info);
		}
	}
	else if(msg_info->cur_state == SPI_SLAVE_MSG_STATE_PROCESS)
	{
		/* OK processwindow match so continue with the rest */
		//msg_info->msg_ptr_in[msg_info->cur_cnt++] = in_char;
		//printf("X!%d\r\n", msg_info->cur_cnt);
		//printf("l:%d\r\n", msg_info->cur_cnt);
		uint16_t cur_cnt = (msg_info->cur_cnt - SPI_SLAVE_MESSAGE_PAYLOAD_LOC);
//		if(cur_cnt ==8 )
//			printf("l:%d\r\n", msg_info->msg_ptr_in[msg_info->cur_cnt]);
		if(cur_cnt>=(msg_info->len_msg-1) || msg_info->cur_cnt>=(msg_info->msg_max_size-1))
		{
//			uint8_t i = 0;
//			//printf("R%d", msg_info->cur_cnt);
//			for(i=0;i<msg_info->len_msg;i++)
//			{
//				printf("%d ", msg_info->msg_ptr_in[i]);
//			}
			reset_values(msg_info);
			/* call the input process function */
			msg_info->process_cb(msg_info);
			msg_info->cur_state = SPI_SLAVE_MSG_STATE_BEGIN;
		}
	}
	msg_info->cur_cnt++;
	msg_info->spi_out_cb(msg_info->msg_ptr_out[msg_info->cur_cnt]);
}

uint16_t spi_slave_message_encode_header(uint8_t *msg_out, uint16_t msg_len, uint8_t type)
{
	uint8_t i = 0;

	for(i=0;i<PROCESS_WINDOW_TRG_SIZE;i++)
	{
		msg_out[i] = process_window_trigger_event[i];
	}

	memcpy(&msg_out[SPI_SLAVE_MESSAGE_LEN_LOC], &msg_len, sizeof(uint16_t));

	memcpy(&msg_out[SPI_SLAVE_MESSAGE_TYPE_LOC], &type, sizeof(uint8_t));

	return SPI_SLAVE_MESSAGE_PAYLOAD_LOC;
}

uint16_t spi_slave_message_encode(uint8_t *msg_out, uint16_t msg_len, uint8_t *msg_in, uint8_t type)
{
	uint16_t chars_written = spi_slave_message_encode_header(msg_out, msg_len, type);

	//memcpy(&msg_out[PROCESS_WINDOW_TRG_SIZE], &type, sizeof(uint8_t));

	if(type == SPI_SLAVE_MESSAGE_TYPE_DUMMY)
	{
		/* Fill message with dummy message */
		memset(&msg_out[chars_written], 0xFF, msg_len);
	}
	else if (type == SPI_SLAVE_MESSAGE_TYPE_NORMAL)
	{
		/* Fill message with payload */
		memcpy(&msg_out[chars_written], msg_in, msg_len);
	}

	return msg_len + chars_written;
}


int16_t spi_slave_message_get_info(spi_slave_message_t *msg_info, uint16_t what)
{
	uint16_t len = 0;

	switch(what)
	{
		case SPI_SLAVE_MESSAGE_INFO_TYPE:
			return msg_info->msg_ptr_in[SPI_SLAVE_MESSAGE_TYPE_LOC];
		break;
		case SPI_SLAVE_MESSAGE_INFO_LEN:
			/* Get len from and of message */
			memcpy(&len, &msg_info->msg_ptr_in[SPI_SLAVE_MESSAGE_LEN_LOC], sizeof(len));
			return len;
		break;
		default:
			return -1;
	}

	return -1;
}

#ifdef __linux__
#include <byteswap.h>

void spi_slave_message_swap_endianess(uint8_t *msg)
{
	uint16_t *tmp_u16;

	/* Addr, len an crc have to be swapped */
	tmp_u16 = (uint16_t *) &msg[SPI_SLAVE_MESSAGE_LEN_LOC];
	*tmp_u16 = __bswap_16(*tmp_u16);
}
#endif

