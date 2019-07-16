#pragma once

#include <stdint.h>

#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

#ifndef bit_sizeof
#define bit_sizeof(type) (sizeof(type) * 8)
#endif

#define NOP asm volatile("nop")

struct ws2812_IO_struct
{
	volatile uint8_t *ws2812_port;
	volatile uint8_t *ws2812_dir;
	uint8_t ws2812_pin;
};

struct ws2812_configuration_struct
{
	struct ws2812_IO_struct ws2812_io_conf;
};

struct __attribute__((packed)) ws2812_pixel
{
	uint8_t B;
	uint8_t R;
	uint8_t G;
};


// Default colors

static const struct ws2812_pixel Black  =	{0, 0, 0};
static const struct ws2812_pixel White	 =	{255, 255, 255};
static const struct ws2812_pixel Red	 =	{0, 255, 0};
static const struct ws2812_pixel Green	 =	{0, 0, 255};
static const struct ws2812_pixel Blue	 =	{255, 0, 0};
static const struct ws2812_pixel Orange =	{0, 255, 165};
static const struct ws2812_pixel Brown	 =	{42, 165, 42};
static const struct ws2812_pixel Purple =	{128, 128, 0};

void ws2812_write_single_pixel(const struct ws2812_pixel *pixel);
void ws2812_write_strip(const struct ws2812_pixel *pixel_buffer,uint8_t length,uint8_t allTheSame);
void ws2812_set_pin(volatile uint8_t *Port,volatile uint8_t *DDR, uint8_t ws2812_pin);
void ws2812_set_configuration_struct(struct ws2812_configuration_struct *configuration);
void ws2812_blink_strip(const struct ws2812_pixel *pixel_bufferA,const struct ws2812_pixel *pixel_bufferB,uint8_t length,uint32_t blink_period,uint8_t allTheSame);

extern uint32_t get_timestamp();



//static inline __attribute__((always_inline)) void ws2812_one(void);
//static inline __attribute__((always_inline)) void ws2812_zero(void);