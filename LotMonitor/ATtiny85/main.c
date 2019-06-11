#ifndef F_CPU
#define F_CPU 16000000UL  // 16MHz for the Attiny85
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "light_ws2812.h"  // https://github.com/cpldcpu/light_ws2812
#include "SRF05_driver.h"
#include "tiny_owi_slave.h"
#include "../common_communication_layer/communication_structs/common_structs.h"
#include "comunications.h"
#include "main.h"

// ######################################### [ Global Defines ] #########################################// 
#define ONBOARD_LED		PB3
#define US_ECHO			PB1
#define US_TRIGGER		PB0
#define OWI_PIN			PB2
//      WS2819_DATA   PB4 // See Makefile WS2812_PIN


#define US_INTERRUPT_DIV		10  
#define US_ECHO_TIMEOUT			517 // 30000us / 58us 
#define US_TRIGGER_TIMER_FREQ	160 // 100Hz
#define US_ECHO_TIMER_FREQ		116 // 58us interruption

#define WS2819_STRIPE_LEN	5
#define WS2819_BRIGHTNESS	100 // percent [optional, comment for full brightness]

#define OWI_SAMPLE_TIME		(60-20)  // 30us // 20
#define OWI_TIME_SLOT		(120-40) // 60us
#define OWI_RESET_PULSE		250 // 120us


// ######################################### [ Global variables definition ] #########################################//  

volatile struct US_struct SRF05_struct;
volatile struct OWI_struct owi_comunication_struct;
uint8_t write_flag = False;


// ######################################### [ ISR definition ] #########################################//  

//---------
// This function is executed 100 times per sec by timer interrupt (See TCCR0B)
//----------
//Timer 0 non blocking interrupt for ultrasonic 
ISR(TIM0_COMPA_vect,ISR_NOBLOCK) 
{	
	us_state_machine_poll(); //polling variable 100hz to 55us
}

// Timer 1 interrupts for OWI 
ISR(TIM1_COMPA_vect)
{
	owi_sample_signal( );; // sample bus 30us
}

ISR(TIM1_COMPB_vect)
{
	owi_time_slot( ); // time slot ends 60us
}

ISR(TIM1_OVF_vect)
{  //60us
}

ISR(INT0_vect)
{
	owi_new_bit();  //data request /new data in
	PINB |= (1 << ONBOARD_LED);
}

// ######################################### [ MAIN ] #########################################//  

int main(void) 
{
//----------
	// ######################################### [ General initialization ] #########################################//  
	
	DDRB   |= (1 << ONBOARD_LED);   // set data direction register for ONBOARD_LED as output
	PORTB &= ~(1 << ONBOARD_LED);
	//DDRB |= (1<<PINB0); //DEBUG output
	  
	struct parking_status_t parking_info;	
	parking_info.distance = UINT16_MAX;
	parking_info.status = parking_free;

	// ######################################### [ Driver struct initialization ] #########################################//
	
	owi_comunication_struct.owi_protocol.owi_address = 1;
	set_owi_struct(&owi_comunication_struct);
	struct communication_buffers buffers;
	set_comunication_buffers(&buffers);
	us_set_us_struct(&SRF05_struct);
	
	// ######################################### [ Driver Pin initialization ] #########################################//
	GIMSK |= (1 << INT0); //OWI pin activate interrupt by INT0 pin
	MCUCR |= (1<<ISC01); // OWI pin interrupt by falling edge
	owi_configure_pin(&PORTB, &DDRB, &PINB, OWI_PIN);
	us_set_pin(&PORTB, &DDRB, &PINB, US_TRIGGER, US_ECHO);
	
	// ######################################### [ Driver Timer initialization ] #########################################//
	
	TCCR0B &= TIM0_NO_CLOCK;	// US timer (timer 0) Avoid start clock
	TCCR0A |= (1 << WGM01);		// US timer (timer 0) counter mode to CTC
	TIMSK  |= (1 << OCIE0A);	// US timer (timer 0) enable Timer CTC interrupt
	us_set_timer(&TCCR0B, &TCNT0, &OCR0A, TIM0_NO_CLOCK, TIM0_PREESCALER_1024, TIM0_PREESCALER_8, US_TRIGGER_TIMER_FREQ, US_ECHO_TIMER_FREQ, US_INTERRUPT_DIV, US_ECHO_TIMEOUT);
	
	OCR1A = OWI_SAMPLE_TIME;	// OWI timer (timer 1) Set compare A
	OCR1B = OWI_TIME_SLOT;		// OWI timer (timer 1) Set compare B
	OCR1C = OWI_TIME_SLOT;		// OWI timer (timer 1) Set compare C
	TIMSK |= ((1 << OCIE1A) | (1 << OCIE1B) | (1 << TOIE1)); // OWI timer (timer 1) interrupt by compare or overflow
	TCCR1 |= (1 << CTC1);		// OWI timer (timer 1) counter mode to CTC
	TCCR1 &= TIM1_NO_CLOCK;		// OWI timer (timer 1) Avoid start clock
	owi_configure_timer(&TCNT1,&TCCR1,TIM1_NO_CLOCK,TIM1_PREESCALER_8);
	
	// ######################################### [ Driver initialization ] #########################################//
	
	us_initialize_state_machine();
	owi_initialize_state_machine();
  

	// ######################################### [ Enable global interrupt ] #########################################//
  
	sei();                          

	// ######################################### [ Main loop ] #########################################//

	while(1)
	{
		if (owi_is_data_ready() == True)
		{	PINB |= (1<<PINB3);
			PINB |= (1<<PINB3);
		
			uint8_t read_data = owi_get_data();
			uint8_t is_packet_ready = process_data_packet(read_data);
		
			if (is_packet_ready == True)
			{
					for (int j=0;j<reinterpret_uint16(buffers.decode_buffer+2);j++)
					{
						PINB |= (1<<PINB3);
						PINB |= (1<<PINB3);
					}

				set_input_data(buffers.decode_buffer,&parking_info);
			}
		}
		else if (write_flag == True)
		{
			uint8_t data;
			if (owi_is_data_set() == True)
			{
				if (get_next_byte(&data) == True)
				{
						owi_set_data(data);
				}
				else
				{
					write_flag = False;
				
				}
			}
		}
	}
}

void set_input_data(uint8_t *packet, struct parking_status_t *parking_info)
{
	//uint8_t CMD = reinterpret_uint8(packet);
	struct packet_struct_t *data_packet = (struct packet_struct_t *) packet;
	switch (data_packet->CMD)
	{
		case parking_status:
		
			parking_info->status	= data_packet->packet.parking.status;
			parking_info->distance	= data_packet->packet.parking.distance;
		
		
	
			for (int j=0;j<parking_info->distance;j++)
			{
				PINB |= (1<<PINB3);
				PINB |= (1<<PINB3);
			}

				
		break;
		
		case distance:
			parking_info->distance	= data_packet->packet.distance;
		 
		break;
		
		case place_status:
		
			//parking_info->status = data_packet->packet.status;
			parking_info->status = parking_reserved;
			write_flag = process_write_packet(&(parking_info->status),sizeof(parking_info->status));
		
		break;
		
		default:
		break;
	}
}