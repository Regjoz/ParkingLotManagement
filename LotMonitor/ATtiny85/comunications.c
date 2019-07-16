#include "comunications.h"
#include "../common_communication_layer/communication_structs/common_structs.h"
#include "../common_communication_layer/Cobs/cobs.h"
#include "tiny_owi_slave.h"
#include <util/crc16.h>




static struct communication_buffers *internal_buffers;

static uint8_t encode_size = 0;
static uint8_t write_size = 0;
static uint8_t byte_count = 0;

static uint8_t write_flag = False;

static uint32_t comunication_timeout = 0;
static uint32_t time_reference = 0;

static void set_new_reference()
{
	time_reference = get_timestamp();
}

void set_comunication_buffers(struct communication_buffers *buffers)
{
	internal_buffers = buffers;
}

void frame_start()
{
	encode_size = 0;
}

uint8_t process_data_packet(uint8_t data)
{
	uint8_t is_packet_processed = False;
	
	if (data == 0)
	{
		
		cobs_decode_result cobs_out =  cobs_decode(internal_buffers->decode_buffer,sizeof(internal_buffers->decode_buffer),internal_buffers->encode_buffer,encode_size);

		if (cobs_out.status == COBS_DECODE_OK)
		{
			is_packet_processed = True;
		}
		encode_size = 0;
	}
	else
	{
		
		internal_buffers->encode_buffer[encode_size] = data;
		encode_size++;
	}
	
	return is_packet_processed;
}

uint8_t process_write_packet(void *data_tosend, uint8_t size)
{	
	uint8_t is_packet_processed = False;
	uint8_t *data_send = (uint8_t *) data_tosend;
	cobs_encode_result cobs_out = cobs_encode(internal_buffers->encode_buffer,sizeof(internal_buffers->encode_buffer),data_send,size) ;
	if (cobs_out.status == COBS_ENCODE_OK)
	{
		write_size = cobs_out.out_len+1;
		byte_count = 0;
		internal_buffers->encode_buffer[cobs_out.out_len] = 0; //frame end
		is_packet_processed = True;
	}
	return is_packet_processed;
}

uint8_t get_next_byte(uint8_t *byte)
{
	uint8_t is_next_byte_available = False;
	if (byte_count < write_size)
	{
		*byte = internal_buffers->encode_buffer[byte_count];
		byte_count++;
		is_next_byte_available = True;
	}
	else
	{
		is_next_byte_available = False;
	}
	
	return is_next_byte_available;
	
}

void comunication_poll_data(void (*onData)(uint8_t *buffers, uint8_t *write_Data))
{
	if (owi_is_data_ready() == True)
	{
		uint8_t read_data = owi_get_data();
		uint8_t is_packet_ready = process_data_packet(read_data);
		
		if (is_packet_ready == True)
		{
			onData(internal_buffers->decode_buffer,&write_flag);
		}
		set_new_reference();
	}
	else if (write_flag == True)
	{
		uint8_t data;
		if (owi_is_data_set() == True)
		{
			if (get_next_byte(&data) == True)
			{
				owi_set_data(data);
				set_new_reference();
			}
			else
			{
				write_flag = False;
				
			}
		}
		
	}
	
	if (owi_is_busy() == True) 
	{
		if (get_timestamp() > (comunication_timeout + time_reference))
		{
			owi_initialize_state_machine(); //timeout reached
			frame_start();
			write_flag = False;
		}
	}
	else
	{
		set_new_reference();
	}
	
}

void set_comunication_timeout(uint32_t timeout)
{
	comunication_timeout = timeout;
}

void rotate_buffer(uint8_t *buffer,uint8_t size)
{
	uint8_t temp_value;
	for(int i = 0; i < (size >> 1); i++)
	{
		temp_value = (*(buffer + i));
		(*(buffer + i)) = (*(buffer + (size - 1) - i));
		(*(buffer + (size - 1) - i)) = temp_value;
	}
}

uint8_t	reinterpret_uint8(uint8_t *buffer)
{
	return (*buffer);
}
uint16_t reinterpret_uint16(uint8_t *buffer)
{
	uint16_t data = (*((uint16_t *)  buffer));
	return data;
}
uint32_t reinterpret_uint32(uint8_t *buffer)
{
	uint32_t data = (*((uint32_t *)  buffer));
	return data;
}
uint64_t reinterpret_uint64(uint8_t *buffer)
{
	uint64_t data = (*((uint64_t *)  buffer));
	return data;
}



 