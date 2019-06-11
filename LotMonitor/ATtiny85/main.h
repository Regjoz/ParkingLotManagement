// cRGB is internal grb !
#pragma once
#include "light_ws2812.h"
#include "../common_communication_layer/communication_structs/common_structs.h"

#define TIM0_NO_CLOCK					(~(1<<CS02 | 1<<CS01 | 1<<CS00))
#define TIM0_PREESCALER_1				1<<CS00
#define TIM0_PREESCALER_8				1<<CS01
#define TIM0_PREESCALER_64				(1<<CS01 | 1<<CS00)
#define TIM0_PREESCALER_256				1<<CS02
#define TIM0_PREESCALER_1024			(1<<CS02 | 1<<CS00)


#define TIM1_NO_CLOCK					(~(1<<CS13 | 1<<CS12 | 1<<CS11 | 1<<CS10))		
#define TIM1_PREESCALER_1				1<<CS10
#define TIM1_PREESCALER_2				1<<CS11
#define TIM1_PREESCALER_4				(1<<CS11 | 1<<CS10)
#define TIM1_PREESCALER_8				1<<CS12
#define TIM1_PREESCALER_16				(1<<CS12 | 1<<CS10)
#define TIM1_PREESCALER_32				(1<<CS12 | 1<<CS11)
#define TIM1_PREESCALER_64				(1<<CS12 | 1<<CS11 | 1<<CS10))
#define TIM1_PREESCALER_128				1<<CS13
#define TIM1_PREESCALER_256				(1<<CS13 | 1<<CS10)
#define TIM1_PREESCALER_512				(1<<CS13 | 1<<CS11)
#define TIM1_PREESCALER_1024			(1<<CS13 | 1<<CS11 | 1<<CS10)
#define TIM1_PREESCALER_2048			(1<<CS13 | 1<<CS12)
#define TIM1_PREESCALER_4096			(1<<CS13 | 1<<CS12 | 1<<CS10)
#define TIM1_PREESCALER_8192			(1<<CS13 | 1<<CS12 | 1<<CS11)
#define TIM1_PREESCALER_16384			(1<<CS13 | 1<<CS12 | 1<<CS11 | 1<<CS10)


static const struct cRGB color_black = {   0,   0,   0 };
static const struct cRGB color_red   = {   0, 255,   0 };
static const struct cRGB color_green = { 255,   0,   0 };
static const struct cRGB color_blue  = {   0,   0, 255 };
static const struct cRGB color_white = { 255, 255, 255 };

void allOFF(uint8_t update_available);
void all(uint8_t r, uint8_t g, uint8_t b);
void allRGB(uint8_t r, uint8_t g, uint8_t b);
void allColor(struct cRGB color);

void set_timer0_preescaler_and_compare_registers(uint8_t preescaler,uint8_t compare);

void set_input_data(uint8_t *packet , struct parking_status_t *parking_info);

#define True 1
#define False 0




