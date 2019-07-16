#include "parking_lot_controller.h"
#include "tiny_ws2812.h"

static inline __attribute__((always_inline)) int line_ecuation(const int x1, const int x2, const int y1, const int y2, int value)
{
	return ((y2 - y1) * 1000 / (x2 - x1) * (value - x1) + y1 * 1000)/1000;
}

static inline __attribute__((always_inline)) int first_step(int value)
{
	int led_length = line_ecuation(x1_first, x2_first, y1_first, y2_first, value);
	return (led_length <= 0) ? 1 : led_length ;
}

static inline __attribute__((always_inline)) int second_step(int value)
{
	int led_length = line_ecuation(x1_second, x2_second, y1_second, y2_second, value);
	return (led_length <= 0) ? 1 : led_length ;
}

static void fill_strip(struct ws2812_pixel *pixel,const struct ws2812_pixel *color, int length, int total_length )
{
	for(int i = 0; i < length; i++)
	{
		*(pixel+i) =  *color;
	}
	
	for(int j = length; j < total_length; j++)
	{
		*(pixel+j) = Black;
	}
}

static void control_leds(struct parking_status_t *parking_info)
{
	
	if (parking_info->distance > far)
	{
		 ws2812_write_strip(&Green, leds_full, True);
	}
	else if (parking_info->distance > medium)
	{
		struct ws2812_pixel led_strip[leds_full];
		int length = first_step(parking_info->distance);
		fill_strip(led_strip,&Green,length,leds_full);
		ws2812_write_strip(led_strip,leds_full,False);
		
	}
	else if (parking_info->distance > close)
	{
		struct ws2812_pixel led_strip[leds_full];
		int length = second_step(parking_info->distance);
		fill_strip(led_strip,&Red,length,leds_full);
		ws2812_write_strip(led_strip,leds_full,False);
	}
	else
	{
		ws2812_blink_strip(&Black,&Red,leds_full,16,True);
	}
}

void parking_controller_poll(struct parking_status_t *parking_info,uint8_t is_busy)
{
	static uint32_t lastTimeStamp = 0;
	
	if (is_busy == False && (lastTimeStamp + 4) <= get_timestamp())
	{
		control_leds(parking_info);
		lastTimeStamp = get_timestamp();
	}
	
}