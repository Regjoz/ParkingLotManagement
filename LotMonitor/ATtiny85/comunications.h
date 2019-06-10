#pragma once

#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

struct __attribute__((packed)) communication_buffers
{
	uint8_t encode_buffer[COBS_ENCODE_DST_BUF_LEN_MAX(sizeof(struct packet_struct_t))+1];
	uint8_t decode_buffer [(sizeof(struct packet_struct_t))];
};


void set_comunication_buffers(struct communication_buffers *buffers);
void frame_start();
uint8_t process_data_packet(uint8_t data);
uint8_t process_write_packet(void *data_tosend, uint8_t size);
uint8_t get_next_byte(uint8_t *byte);
void rotate_buffer(uint8_t *buffer,uint8_t size);

uint8_t		reinterpret_uint8(uint8_t *buffer);
uint16_t	reinterpret_uint16(uint8_t *buffer);
uint32_t	reinterpret_uint32(uint8_t *buffer);
uint64_t	reinterpret_uint64(uint8_t *buffer);

