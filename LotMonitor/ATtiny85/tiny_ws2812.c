#include "tiny_ws2812.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static struct ws2812_configuration_struct *ws2812_conf;
static uint32_t timestamp = 0;

static inline __attribute__((always_inline)) void ws2812_one(void)
{
	//*ws2812_conf->ws2812_io_conf.ws2812_port |= (1 << ws2812_conf->ws2812_io_conf.ws2812_pin); //force 1
	PINB = (1 << PINB3);
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	PINB = (1 << PINB3);
	//*ws2812_conf->ws2812_io_conf.ws2812_port &= ~(1 << ws2812_conf->ws2812_io_conf.ws2812_pin); //force 0
	//toggle pin
}

static inline __attribute__((always_inline)) void ws2812_zero(void)
{
	//*ws2812_conf->ws2812_io_conf.ws2812_port |= (1 << ws2812_conf->ws2812_io_conf.ws2812_pin); //force 1
	PINB = (1 << PINB3);
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	PINB = (1 << PINB3);
	//*ws2812_conf->ws2812_io_conf.ws2812_port &= ~(1 << ws2812_conf->ws2812_io_conf.ws2812_pin); //force 0
	
}

void ws2812_set_configuration_struct(struct ws2812_configuration_struct *configuration)
{
	ws2812_conf = configuration;
}

void ws2812_set_pin(volatile uint8_t *Port,volatile uint8_t *DDR, uint8_t ws2812_pin)
{
	ws2812_conf->ws2812_io_conf.ws2812_dir = DDR;
	ws2812_conf->ws2812_io_conf.ws2812_pin = ws2812_pin;
	ws2812_conf->ws2812_io_conf.ws2812_port = Port;
	
	*ws2812_conf->ws2812_io_conf.ws2812_dir |= (1 << ws2812_conf->ws2812_io_conf.ws2812_pin);  //OWI pin as output, writing to ws2812
	*ws2812_conf->ws2812_io_conf.ws2812_port &= ~(1 << ws2812_conf->ws2812_io_conf.ws2812_pin); //force 0
	//ws2812_write_single_pixel(&Off);
}



void ws2812_write_single_pixel(const struct ws2812_pixel *pixel)
{
	uint32_t *pixel_array = (uint32_t *) pixel;
	for (uint8_t i = 0; i  < bit_sizeof(struct ws2812_pixel); i++)
	{
		if(((*pixel_array << i) & 0x800000) == 0x800000) //bit shift left if one send one if zero send zero 8 bit for one color
		{ 
			ws2812_one();
		}
		else
		{
			ws2812_zero();
		}
	}
}

void ws2812_write_strip(const struct ws2812_pixel *pixel_buffer,uint8_t length,uint8_t allTheSame)
{
	*ws2812_conf->ws2812_io_conf.ws2812_port &= ~(1 << ws2812_conf->ws2812_io_conf.ws2812_pin); //force 0
	cli();
	for (uint8_t i = 0; i < length; i++)
	{
		const struct ws2812_pixel *_pixel = (allTheSame == True) ? pixel_buffer : pixel_buffer + i;
		ws2812_write_single_pixel(_pixel);
	}
	sei();
	
}

void ws2812_blink_strip(const struct ws2812_pixel *pixel_bufferA,const struct ws2812_pixel *pixel_bufferB,uint8_t length,uint32_t blink_period,uint8_t allTheSame)
{
	static uint32_t total_blink_time = 0;
	static uint32_t blink_time = 0;
	
	
	if (timestamp < blink_time)
	{
		ws2812_write_strip(pixel_bufferA,length,allTheSame);
	} 
	else if(timestamp < total_blink_time)
	{
		ws2812_write_strip(pixel_bufferB,length,allTheSame);
	}
	else
	{
		uint32_t half_blink_period = blink_period >> 1;
		total_blink_time = get_timestamp() + blink_period;
		blink_time = get_timestamp() + half_blink_period;
	}
	
	timestamp = get_timestamp();
	
}
