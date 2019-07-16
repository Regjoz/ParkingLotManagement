#include "tiny_owi_slave.h"
#ifdef OWI_DEBUG
#include <avr/io.h>
#endif

#include <avr/interrupt.h>

static volatile struct OWI_struct *data_struct;
static uint8_t sync_disable = False;

static inline __attribute__((always_inline)) void owi_stop_timer()
{
	*data_struct->owi_timer_conf.owi_timer_clock &= data_struct->owi_timer_conf.owi_stop_timer; //syncronize timer
	*data_struct->owi_timer_conf.owi_timer_counter = 0;
}

static inline __attribute__((always_inline)) void owi_start_timer()
{
	*data_struct->owi_timer_conf.owi_timer_clock |= data_struct->owi_timer_conf.owi_timer_preescaler;
}

static inline __attribute__((always_inline)) uint8_t owi_read_pin()
{
	uint8_t pin_status = (*data_struct->owi_io_conf.owi_read_register);
	pin_status &= (1 << data_struct->owi_io_conf.owi_pin);
	pin_status = (pin_status == (1 << data_struct->owi_io_conf.owi_pin));
	return pin_status;
}

static inline __attribute__((always_inline)) void owi_clean_buffer()
{
	data_struct->owi_protocol.owi_read_intermediate_buffer = 0;
	
}
static inline __attribute__((always_inline)) void owi_clean_counter()
{
	data_struct->owi_statemachine.time_slots_counter = 0;
	data_struct->owi_statemachine.total_time_slots_elapsed = 0;
}

static inline __attribute__((always_inline)) void owi_clean_address_counter()
{
	data_struct->owi_protocol.owi_address_in = 0;
	data_struct->owi_protocol.owi_address_byte_counter = sizeof(data_struct->owi_protocol.owi_address_in)-1;
}

static uint8_t owi_fill_intermediate_buffer(uint8_t pin_status)
{
	uint8_t is_intermediate_buffer_full = False;
	
	if (data_struct->owi_statemachine.total_time_slots_elapsed > 2 && (pin_status == False)) //more than two time slots without update pin
	{
			data_struct->owi_statemachine.time_slots_counter = data_struct->owi_statemachine.total_time_slots_elapsed;
			data_struct->owi_statemachine.total_time_slots_elapsed = 0;
			data_struct->owi_statemachine.STM_states = reset;
	} 
	else
	{
		switch(data_struct->owi_statemachine.STM_trigger)
		{
			case new_bit:
				
					data_struct->owi_statemachine.total_time_slots_elapsed = 0;
				
				break;
			
			case sample_signal:
			
				if(data_struct->owi_statemachine.total_time_slots_elapsed == 0) // only sample if new bit present
				{
					//// PINB |= (pin_status<<// PINB0); //DEBUGG
					data_struct->owi_protocol.owi_read_intermediate_buffer |= (pin_status << (data_struct->owi_statemachine.time_slots_counter)); //move bit to required position
				}
			
			break;
			
			case time_slot:
			
				if (data_struct->owi_statemachine.total_time_slots_elapsed == 0) // only set if a sample was performed
				{
					//// PINB |= (1<<// PINB0);
					if (data_struct->owi_statemachine.time_slots_counter < (bit_sizeof(data_struct->owi_protocol.owi_read_intermediate_buffer)-1))
					{
						data_struct->owi_statemachine.time_slots_counter++;
					}
					else
					{
						is_intermediate_buffer_full = True;
					}
				}
				if (is_intermediate_buffer_full == False)
				{
					owi_start_timer();
					data_struct->owi_statemachine.total_time_slots_elapsed++;
				}
				
			break;
			
			default:
			break;
		}
	}
	
	
	return is_intermediate_buffer_full;
}

static void owi_state_machine_poll(uint8_t pin_status)
{
	
	switch(data_struct->owi_statemachine.STM_states)
	{
		case start:
		
			owi_clean_buffer();
			owi_clean_counter();
			owi_clean_address_counter();
			*data_struct->owi_io_conf.owi_dir &= ~(1 << data_struct->owi_io_conf.owi_pin);  //OWI pin as input, listening to bus
			data_struct->owi_protocol.owi_is_data_write = True;
			data_struct->owi_protocol.owi_is_busy = False;
			switch (data_struct->owi_statemachine.STM_trigger)
			{
				case new_bit:
					
						data_struct->owi_statemachine.STM_states = reset;
				break;
				default:
					owi_stop_timer();
					data_struct->owi_statemachine.STM_states = start;
				break;
			}
			
		break;
		
		case reset:		
			data_struct->owi_protocol.owi_is_busy = True;
			switch (data_struct->owi_statemachine.STM_trigger)
			{
				case time_slot:
					owi_start_timer();
					data_struct->owi_statemachine.time_slots_counter++;
				case sample_signal:
					if (pin_status == True)
					{
						if (data_struct->owi_statemachine.time_slots_counter >= 7) //minimun 7 or more time slots for reset
						{
							owi_stop_timer();
							owi_start_timer();
							data_struct->owi_statemachine.time_slots_counter = 0;
							owi_clean_buffer();
							owi_clean_counter();
							owi_clean_address_counter();
							data_struct->owi_protocol.owi_is_data_write = True;
							*data_struct->owi_io_conf.owi_dir |= (1 << data_struct->owi_io_conf.owi_pin);  //OWI pin as output, writing to bus
							*data_struct->owi_io_conf.owi_port &= ~(1 << data_struct->owi_io_conf.owi_pin); //force 0
							data_struct->owi_statemachine.STM_states = presence; // 7 or more time slots of reset signal as protocol define
						}
						else
						{
							data_struct->owi_statemachine.STM_states = start; // reset signal too short something went wrong , or bus is busy
						}
					}
				break;
				/*case sample_signal:
					if (pin_status == True)
					{
						if (data_struct->owi_statemachine.time_slots_counter >= 7) //minimun 7 or more time slots for reset
						{
							owi_stop_timer();
							owi_start_timer();
							data_struct->owi_statemachine.time_slots_counter = 0;
							owi_clean_buffer();
							owi_clean_counter();
							owi_clean_address_counter();
							data_struct->owi_statemachine.STM_states = presence; // 7 or more time slots of reset signal as protocol define
						}
						else
						{
							data_struct->owi_statemachine.STM_states = start; // reset signal too short something went wrong , or bus is busy
						}
					}
				break;*/
				default:
				break;
			}
					
		break;
		
		case presence:
			
			if(data_struct->owi_statemachine.STM_trigger == time_slot) //after 60us change to read again
			{
				
				if (data_struct->owi_statemachine.time_slots_counter == 0)
				{
					owi_start_timer();
					//PCMSK &= ~(1 << data_struct->owi_io_conf.owi_pin); //only react in owi pin //disactivate interrupt by pin change
					//*data_struct->owi_io_conf.owi_dir |= (1 << data_struct->owi_io_conf.owi_pin);  //OWI pin as output, writing to bus
					//*data_struct->owi_io_conf.owi_port &= ~(1 << data_struct->owi_io_conf.owi_pin); //force 0
					data_struct->owi_statemachine.time_slots_counter++;
					
				} 
				else
				{
					*data_struct->owi_io_conf.owi_dir &= ~(1 << data_struct->owi_io_conf.owi_pin);  //OWI pin as input, listening to bus
					data_struct->owi_statemachine.STM_states = rom_command;
					data_struct->owi_statemachine.time_slots_counter = 0;
					//PCMSK |= (1 << data_struct->owi_io_conf.owi_pin); //activate interrupt by pin change
					owi_stop_timer();
					
				}
			}
		
		
		break;
		
		case rom_command:
		
			if (owi_fill_intermediate_buffer(pin_status) == True)
			{	

				data_struct->owi_protocol.ROM_CMD = data_struct->owi_protocol.owi_read_intermediate_buffer;

				owi_clean_buffer();
				owi_clean_counter();

				switch(data_struct->owi_protocol.ROM_CMD)
				{
					case match_rom:
						data_struct->owi_statemachine.STM_states = rom_address; //receive address
					break;
						
					case skip_rom:
					//data_struct->owi_statemachine.STM_states = read; //this command skip address
					break;
						
					default:
						data_struct->owi_statemachine.STM_states = start; // command not implemented or incorrect
					break;
				}
				
			}

		
		break;
		
		case rom_address:
		
			if (owi_fill_intermediate_buffer(pin_status) == True)
			{		
				owi_stop_timer();
			
					
				uint8_t *address = (uint8_t *) &data_struct->owi_protocol.owi_address_in;
				*(address+data_struct->owi_protocol.owi_address_byte_counter) = data_struct->owi_protocol.owi_read_intermediate_buffer;
				data_struct->owi_protocol.owi_address_byte_counter--;
									
				owi_clean_buffer();
				owi_clean_counter();
				
				if (data_struct->owi_protocol.owi_address_byte_counter < 0)
				{

					if (data_struct->owi_protocol.owi_address == data_struct->owi_protocol.owi_address_in)
					{
						data_struct->owi_statemachine.STM_states = read;
					}
					else
					{
						data_struct->owi_statemachine.STM_states = start;
					}
					
					owi_clean_address_counter();
					owi_clean_buffer();
					owi_clean_counter();
				}
					
			}


		break;
		
		case read:
		
			if (owi_fill_intermediate_buffer(pin_status) == True)
			{
				owi_stop_timer();
				data_struct->owi_protocol.owi_read_buffer = data_struct->owi_protocol.owi_read_intermediate_buffer;
				data_struct->owi_protocol.owi_is_data_ready = True;
				owi_clean_buffer();
				owi_clean_counter();
			}
		
		break;
		
		case write:
		
		if (data_struct->owi_statemachine.total_time_slots_elapsed > 2 && (pin_status == False)) //more than two time slots without update pin
		{
			data_struct->owi_statemachine.time_slots_counter = data_struct->owi_statemachine.total_time_slots_elapsed;
			data_struct->owi_statemachine.total_time_slots_elapsed = 0;
			data_struct->owi_statemachine.STM_states = reset;
		}
		else
		{
			switch(data_struct->owi_statemachine.STM_trigger)
			{
				case new_bit:
					
					if (((data_struct->owi_protocol.owi_write_data >> data_struct->owi_statemachine.time_slots_counter) & 0x01) == 0x01) //uno
					{
						//nothing to do 
					}
					else //cero
					{
						*data_struct->owi_io_conf.owi_dir |= (1 << data_struct->owi_io_conf.owi_pin);  //OWI pin as output, writing to bus
						*data_struct->owi_io_conf.owi_port &= ~(1 << data_struct->owi_io_conf.owi_pin); //force 0
					}
					data_struct->owi_statemachine.total_time_slots_elapsed = 0;
					data_struct->owi_protocol.owi_is_data_write = False;
					sync_disable = True;
				break;
				
				//case sample_signal:
				case sample_signal:
					
					if (data_struct->owi_statemachine.total_time_slots_elapsed == 0)
					{
						*data_struct->owi_io_conf.owi_dir &= ~(1 << data_struct->owi_io_conf.owi_pin);  //OWI pin as input, listening to bus
						if (data_struct->owi_statemachine.time_slots_counter < (bit_sizeof(data_struct->owi_protocol.owi_write_data)-1))
						{
							data_struct->owi_statemachine.time_slots_counter++;
							owi_start_timer();
						}
						else
						{
							data_struct->owi_protocol.owi_is_data_write = True;
							owi_clean_counter();
						}
					}
					
					sync_disable = False;
				break;
				
				case time_slot:
				if (data_struct->owi_protocol.owi_is_data_write == False)
				{
					owi_start_timer();
					data_struct->owi_statemachine.total_time_slots_elapsed++;
				}
				break;
				
				default:
				break;
				
			}
		}
		
		
		break;
		
	}
	
}


void owi_configure_pin(volatile uint8_t *Port,volatile uint8_t *DDR,volatile uint8_t *read_register ,uint8_t Pin)
{
	data_struct->owi_io_conf.owi_port = Port;
	data_struct->owi_io_conf.owi_dir = DDR;
	data_struct->owi_io_conf.owi_pin = Pin;
	data_struct->owi_io_conf.owi_read_register = read_register;
	
	*data_struct->owi_io_conf.owi_dir &= ~(1 << data_struct->owi_io_conf.owi_pin);  //OWI pin as input, listening to bus
	
	data_struct->owi_protocol.owi_is_data_ready = False;
	data_struct->owi_protocol.owi_read_buffer = 0;
	data_struct->owi_protocol.owi_read_intermediate_buffer = 0;
}

void owi_configure_timer(volatile uint8_t *timer_counter,volatile uint8_t *timer_clock,uint8_t timer_stop,uint8_t timer_preescaler)
{
	data_struct->owi_timer_conf.owi_timer_counter = timer_counter;
	data_struct->owi_timer_conf.owi_timer_clock = timer_clock;
	data_struct->owi_timer_conf.owi_stop_timer = timer_stop;
	data_struct->owi_timer_conf.owi_timer_preescaler = timer_preescaler;
}

void owi_time_slot()
{
	data_struct->owi_statemachine.STM_trigger = time_slot;
	owi_stop_timer();
	owi_state_machine_poll(owi_read_pin());
}

void owi_sample_signal()
{
	data_struct->owi_statemachine.STM_trigger = sample_signal;	
	owi_state_machine_poll(owi_read_pin());
}

void owi_new_bit()
{
	data_struct->owi_statemachine.STM_trigger = new_bit;
	if (sync_disable == False)
	{
		owi_stop_timer(); //synchronization
		owi_start_timer();
	}

	owi_state_machine_poll(False);
}

void owi_initialize_state_machine()
{
	data_struct->owi_statemachine.STM_states = start;
	data_struct->owi_statemachine.STM_trigger = startup;
	owi_state_machine_poll(True);
}

void set_owi_struct(volatile struct OWI_struct *data_struct_set)
{
	data_struct = data_struct_set;
}



uint8_t owi_is_data_ready()
{
	return data_struct->owi_protocol.owi_is_data_ready;
}

uint8_t owi_get_data()
{
	data_struct->owi_protocol.owi_is_data_ready = False;
	return data_struct->owi_protocol.owi_read_buffer;
}

void owi_set_data(uint8_t data)
{
	data_struct->owi_statemachine.STM_states = write;
	data_struct->owi_protocol.owi_write_data = data;
	data_struct->owi_protocol.owi_is_data_write = False;
}

uint8_t owi_is_data_set()
{
	return data_struct->owi_protocol.owi_is_data_write;
}

uint8_t owi_is_busy()
{
	return data_struct->owi_protocol.owi_is_busy;
}




