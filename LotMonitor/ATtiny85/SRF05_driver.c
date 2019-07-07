#include "SRF05_driver.h"


volatile struct US_struct *US_values;

static inline __attribute__((always_inline)) void us_stop_timer()
{
	*US_values->us_timer.timer_handler &= US_values->us_timer.stop_timer;
	*US_values->us_timer.timer_counter = 0;
}

static inline __attribute__((always_inline)) void us_update_timer(uint8_t preescaler, uint8_t compare)
{
	us_stop_timer();
	*US_values->us_timer.timer_compare = compare;
	*US_values->us_timer.timer_handler |= preescaler;
}

static inline __attribute__((always_inline)) uint8_t us_read_pin()
{
	uint8_t pin_status = (*US_values->us_io.us_read_register);
	pin_status &= (1 << US_values->us_io.us_echo_pin);
	pin_status = (pin_status == (1 << US_values->us_io.us_echo_pin));
	return pin_status;
}


void us_state_machine_poll()
{
	switch(US_values->us_statemachine.state)
	{
		case trigger:
		
			if(US_values->us_statemachine.trigger_elapsed >= US_values->us_timer.frequency_divider )  // software divisor to trigger measure every 10 ms
			{
				*US_values->us_io.us_port |= (1 << US_values->us_io.us_trigger_pin);
				us_update_timer(US_values->us_timer.preescaler_high_speed,US_values->us_timer.count_value_high_speed); //set prescaler to 8 (CLK=16MHz/8/116=97.17241Hz, 58us)
				US_values->us_statemachine.state = waiting_for_echo;
			}
			else
			{
				US_values->us_statemachine.trigger_elapsed++ ;
			}
		break;
		
		case waiting_for_echo:
			*US_values->us_io.us_port &= ~(1 << US_values->us_io.us_trigger_pin);  //deactivate trigger pin and wait for echo signal. Maximum waiting time 30 ms
			if(us_read_pin() == False)
			{
				if(US_values->us_statemachine.echo_elapsed >= US_values->us_statemachine.echo_timeout)
				{
				
					US_values->us_statemachine.state = time_out;
				}
				else
				{
					US_values->us_statemachine.echo_elapsed++;
				}
			}
			else
			{
				US_values->us_statemachine.state = echo_received;
			}
		break;
		
		case echo_received:
			//PINB |= (1 << ONBOARD_LED);  //With logic analyzer you can check that the measurement are corrects connecting in PB1
			US_values->us_statemachine.echo_total_time++;
		
			if(US_values->us_statemachine.echo_total_time < US_values->us_statemachine.echo_timeout)
			{
				if(us_read_pin() == False)
				{
					US_values->distance = US_values->us_statemachine.echo_total_time;
					us_initialize_state_machine();
				}
			}
			else
			{
				US_values->us_statemachine.state = time_out;
			}
		
		break;
		
		case time_out:
		
			us_initialize_state_machine();
			US_values->distance = UINT16_MAX;
		
		break;
	}
}

uint16_t us_get_distance()
{
	return US_values->distance;
}

void us_initialize_state_machine()
{
	US_values->us_statemachine.echo_elapsed		= 0;
	US_values->us_statemachine.state			= trigger;
	US_values->us_statemachine.trigger_elapsed	= 0;
	US_values->us_statemachine.echo_total_time	= 0;
	us_update_timer(US_values->us_timer.preescaler_low_speed,US_values->us_timer.count_value_low_speed);
	//US_values->distance = UINT16_MAX;
}

void us_set_us_struct(volatile struct US_struct *US_data)
{
	US_values = US_data;
	US_values->distance = UINT16_MAX;
}

void us_set_timer(volatile uint8_t *timer_handler, volatile uint8_t *timer_counter, volatile uint8_t *timer_compare, uint8_t stop_timer, uint8_t preescaler_low_speed, uint8_t preescaler_high_speed, uint8_t count_value_low_speed, uint8_t count_value_high_speed, uint16_t divider,  uint16_t echo_timeout)
{
	US_values->us_timer.count_value_high_speed	=	count_value_high_speed;
	US_values->us_timer.count_value_low_speed	=	count_value_low_speed;
	
	US_values->us_timer.preescaler_high_speed	=	preescaler_high_speed;
	US_values->us_timer.preescaler_low_speed	=	preescaler_low_speed;
	
	US_values->us_timer.stop_timer				=	stop_timer;
	
	US_values->us_timer.timer_compare			=	timer_compare;
	US_values->us_timer.timer_counter			=	timer_counter;
	US_values->us_timer.timer_handler			=	timer_handler;
	
	US_values->us_timer.frequency_divider		=	divider;
	
	US_values->us_statemachine.echo_timeout		=   echo_timeout;
	
}

void us_set_pin(volatile uint8_t *Port, volatile uint8_t *DDR, volatile uint8_t *read_register, uint8_t trigger_pin, uint8_t echo_pin)
{
	US_values->us_io.us_port			= Port;
	US_values->us_io.us_dir				= DDR;
	US_values->us_io.us_trigger_pin		= trigger_pin;
	US_values->us_io.us_echo_pin		= echo_pin;
	US_values->us_io.us_read_register	= read_register;
	
	*US_values->us_io.us_dir &= ~(1 << US_values->us_io.us_echo_pin);   // set data direction register for HY-SRF05 echo port
	*US_values->us_io.us_port &= ~(1 << US_values->us_io.us_echo_pin);  // No internal pullup
	*US_values->us_io.us_dir |= (1 << US_values->us_io.us_trigger_pin); // set data direction register for HY-SRF05 trigger port
}


