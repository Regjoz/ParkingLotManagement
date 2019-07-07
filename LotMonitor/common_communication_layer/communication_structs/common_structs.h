#pragma once

#include <stdint.h>

enum __attribute__((packed)) write_command
{
	get_parking_info,
	get_distance,
	get_place_status,
	set_parking_status,
	//include here all the needed commands
};

enum __attribute__((packed)) parking_state
{
	parking_free,
	parking_occupied,
	parking_reserved,	
};



struct __attribute__((packed)) parking_status_t
{
	enum parking_state real_status;
	enum parking_state programmed_status;
	uint16_t distance;	
};

//insert here a new struct


union __attribute__((packed)) data_packet
{
	struct parking_status_t parking;
	enum parking_state real_status;
	enum parking_state programmed_status;
	uint16_t distance;
	//insert here the struct definition
};



struct __attribute__((packed)) packet_struct_t
{
	enum write_command CMD;
	union data_packet packet; //buffer size will increment automatically in compile time if you add new struct or data
};









