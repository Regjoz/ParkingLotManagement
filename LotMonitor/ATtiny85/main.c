#ifndef F_CPU
#define F_CPU 16000000UL  // 16MHz for the Attiny85
#endif

#include "main.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "SRF05_driver.h"
#include "tiny_owi_slave.h"
#include "../common_communication_layer/communication_structs/common_structs.h"
#include "comunications.h"
#include "tiny_ws2812.h"
#include "parking_lot_controller.h"


// ######################################### [ Global Defines ] #########################################// 
//#define ONBOARD_LED		PB1
#define US_ECHO			PB1
#define US_TRIGGER		PB0
#define OWI_PIN			PB2
#define WS2812_DATA		PB3


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
volatile uint32_t WDT_timer_counter = 0;
struct parking_status_t parking_info;	
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
}

//WDT interrupt for timestamp
ISR(WDT_vect,ISR_NOBLOCK)
{
	WDT_timer_counter++;
	WDTCR |= (1<<WDCE | 1<<WDE);//set WDT as writable, enable it
	WDTCR |= (1<<WDIE); //activate interrupt
}



// ######################################### [ MAIN ] #########################################//  

int main(void)
{
//----------
	// ######################################### [ General initialization ] #########################################//  
	  
	
	parking_info.distance = UINT16_MAX;
	parking_info.real_status = parking_free;

	// ######################################### [ Driver struct initialization ] #########################################//
	
	owi_comunication_struct.owi_protocol.owi_address = 1;
	set_owi_struct(&owi_comunication_struct);
	
	struct communication_buffers buffers;
	set_comunication_buffers(&buffers);
	
	us_set_us_struct(&SRF05_struct);
	
	struct ws2812_configuration_struct ws2812_struct;
	ws2812_set_configuration_struct(&ws2812_struct);
	
	
	// ######################################### [ Driver Pin initialization ] #########################################//
	GIMSK |= (1 << INT0); //OWI pin activate interrupt by INT0 pin
	MCUCR |= (1<<ISC01); // OWI pin interrupt by falling edge
	owi_configure_pin(&PORTB, &DDRB, &PINB, OWI_PIN);
	
	us_set_pin(&PORTB, &DDRB, &PINB, US_TRIGGER, US_ECHO);
	
	ws2812_set_pin(&PORTB, &DDRB, WS2812_DATA);
	
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
	
	// ######################################### [ WDT Timer initialization ] #########################################//
	
	wdt_reset(); // WDT reset
	MCUSR = 0x00; // clean interrupt register
	WDTCR |= (1<<WDCE | 1<<WDE);//set WDT as writable, enable it
	WDTCR |= (1<<WDIE | WDT_PREESCALER_125ms); //activate interrupt and set new period
	
	// ######################################### [ Driver initialization ] #########################################//
	
	us_initialize_state_machine();
	owi_initialize_state_machine();
	ws2812_write_strip(&Orange,WS2819_STRIPE_LEN,True);
	

	// ######################################### [ Enable global interrupt ] #########################################//
  
	sei();                          

	// ######################################### [ Main loop ] #########################################//

	while(1)
	{
		comunication_poll_data(&set_input_data);
		parking_info.distance = us_get_distance();
		ws2812_blink_strip(&Green,&Red,WS2819_STRIPE_LEN,8,True);
		parking_controller_poll(&parking_info);
		_delay_ms(50);
	}
}

void set_input_data(uint8_t *packet, uint8_t *write_flag)
{
	//uint8_t CMD = reinterpret_uint8(packet);
	struct packet_struct_t *data_packet = (struct packet_struct_t *) packet;
	switch (data_packet->CMD)
	{
		case get_parking_info:
		
			*write_flag = process_write_packet(&parking_info,sizeof(parking_info));
				
		break;
		
		case get_distance:
			*write_flag = process_write_packet(&(parking_info.distance),sizeof(parking_info.distance));
		 
		break;
		
		case get_place_status:
		
			*write_flag = process_write_packet(&(parking_info.real_status),sizeof(parking_info.real_status));
		
		break;
		
		case set_parking_status:
		
			parking_info.programmed_status = data_packet->packet.programmed_status;
		
		break;
		
		default:
		break;
	}
}


uint32_t get_timestamp()
{
	return WDT_timer_counter;
}