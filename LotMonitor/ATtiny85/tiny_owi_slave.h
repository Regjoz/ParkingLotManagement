#pragma once

#include <stdint.h>

#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

#define bit_sizeof(type) (sizeof(type)*8)
#include <avr/io.h> //DEBUUUUUUG
struct OWI_IO_struct
{
	volatile uint8_t *owi_port;
	volatile uint8_t *owi_dir;
	volatile uint8_t *owi_read_register;
	uint8_t owi_pin;
};

struct OWI_TIMER_struct
{
	volatile uint8_t *owi_timer_counter;
	volatile uint8_t *owi_timer_clock;
	uint8_t owi_stop_timer;
	uint8_t owi_timer_preescaler;
};

enum ROM_COMMANDS
{
	search_rom	= 0xf0,
	read_rom	= 0x33,
	match_rom	= 0x55,
	skip_rom	= 0xcc,
	
};

struct OWI_PROTOCOL_struct
{
	enum ROM_COMMANDS ROM_CMD;
	uint64_t owi_address;
	uint64_t owi_address_in;
	uint8_t owi_read_intermediate_buffer;
	uint8_t owi_read_buffer;
	uint8_t owi_write_data;
	//uint8_t *owi_write_buffer;
	//uint8_t owi_write_length;
	uint8_t owi_is_data_write;
	uint8_t owi_is_data_ready;
	uint8_t owi_is_busy;
	int8_t owi_address_byte_counter;	
};

enum state_machine_trigger
{
	none,
	startup,
	new_bit,
	sample_signal,
	time_slot,	
};

enum state_machine_states
{
	start,
	reset,
	presence,
	rom_command,
	rom_address,
	write,
	read,
};

struct OWI_STATEMACHINE_struct
{
	enum state_machine_trigger STM_trigger;
	enum state_machine_states STM_states;
	int8_t time_slots_counter;
	int8_t total_time_slots_elapsed;
};


struct OWI_struct
{
	struct OWI_IO_struct owi_io_conf;
	struct OWI_TIMER_struct owi_timer_conf;
	struct OWI_PROTOCOL_struct owi_protocol;
	struct OWI_STATEMACHINE_struct owi_statemachine;
	
};

void owi_configure_pin(volatile uint8_t *Port,volatile uint8_t *DDR,volatile uint8_t *read_register,uint8_t Pin);
void owi_configure_timer(volatile uint8_t *timer_counter,volatile uint8_t *timer_clock,uint8_t timer_stop,uint8_t timer_preescaler);
void owi_time_slot();
void owi_sample_signal();
void owi_new_bit();
//static inline __attribute__((always_inline)) void owi_stop_timer();
//static inline __attribute__((always_inline)) void owi_start_timer();
//static inline __attribute__((always_inline)) uint8_t owi_read_pin();
void set_owi_struct(volatile struct OWI_struct *data_struct);
void owi_initialize_state_machine();
//static void owi_state_machine_poll(uint8_t pin_status);
//static uint8_t owi_fill_intermediate_buffer(uint8_t pin_status);
//static inline __attribute__((always_inline)) void owi_clean_buffer();
//static inline __attribute__((always_inline)) void owi_clean_counter();
//static inline __attribute__((always_inline)) void owi_clean_address_counter();
uint8_t owi_is_data_ready();
uint8_t owi_is_busy();
uint8_t owi_get_data();
void owi_set_data(uint8_t data);
uint8_t owi_is_data_set();