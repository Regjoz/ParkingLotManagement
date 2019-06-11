#pragma once

#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

enum US_states
{
	trigger,
	waiting_for_echo,
	echo_received,
	time_out,
};

struct US_TIMER_struct
{
	volatile uint8_t *timer_handler;
	volatile uint8_t *timer_counter;
	volatile uint8_t *timer_compare;
	uint8_t stop_timer;
	uint8_t preescaler_low_speed;
	uint8_t preescaler_high_speed;
	uint8_t count_value_low_speed;
	uint8_t count_value_high_speed;
	uint8_t frequency_divider;
};

struct US_IO_struct
{
	volatile uint8_t *us_port;
	volatile uint8_t *us_dir;
	volatile uint8_t *us_read_register;
	uint8_t us_trigger_pin;
	uint8_t us_echo_pin;
};

struct US_STATEMACHINE_struct
{
	uint16_t trigger_elapsed;
	uint16_t echo_elapsed;
	uint16_t echo_total_time;
	uint16_t echo_timeout;
	enum US_states state;
};

struct US_struct
{
	uint16_t distance;
	struct US_STATEMACHINE_struct us_statemachine;
	struct US_TIMER_struct us_timer;
	struct US_IO_struct us_io;
};

void us_state_machine_poll();
uint16_t us_get_distance();
void us_initialize_state_machine();
void us_set_us_struct(volatile struct US_struct *US_data);
void us_set_timer(volatile uint8_t *timer_handler, volatile uint8_t *timer_counter, volatile uint8_t *timer_compare, uint8_t stop_timer, uint8_t preescaler_low_speed, uint8_t preescaler_high_speed, uint8_t count_value_low_speed, uint8_t count_value_high_speed, uint16_t divider,  uint16_t echo_timeout);
void us_set_pin(volatile uint8_t *Port, volatile uint8_t *DDR, volatile uint8_t *read_register, uint8_t trigger_pin, uint8_t echo_pin);
static inline __attribute__((always_inline)) void us_stop_timer();
static inline __attribute__((always_inline)) void us_update_timer(uint8_t preescaler, uint8_t compare);
static inline __attribute__((always_inline)) uint8_t us_read_pin();
