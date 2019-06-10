#pragma once

enum __attribute__((packed)) write_command
{
	parking_status,
	distance,
	place_status,
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
	enum parking_state status;
	uint16_t distance;	
};

//insert here a new struct


union __attribute__((packed)) data_packet
{
	struct parking_status_t parking;
	enum parking_state status;
	uint16_t distance;
	//insert here the struct definition
};



struct __attribute__((packed)) packet_struct_t
{
	enum write_command CMD;
	union data_packet packet; //buffer size will increment automatically in compile time if you add new struct or data
};









