
#pragma once

#include "../common_communication_layer/communication_structs/common_structs.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>

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

#define WDT_PREESCALER_15ms				WDTO_15MS
#define WDT_PREESCALER_30ms				WDTO_30MS
#define WDT_PREESCALER_60ms				WDTO_60MS
#define WDT_PREESCALER_120ms			WDTO_120MS
#define WDT_PREESCALER_250ms			WDTO_250MS
#define WDT_PREESCALER_500ms			WDTO_500MS
#define WDT_PREESCALER_1000ms			WDTO_1S
#define WDT_PREESCALER_2000ms			WDTO_2S
#define WDT_PREESCALER_4000ms			WDTO_4S
#define WDT_PREESCALER_8000ms			WDTO_8S

void set_timer0_preescaler_and_compare_registers(uint8_t preescaler,uint8_t compare);

void set_input_data(uint8_t *packet , uint8_t *write_flag);
uint32_t get_timestamp();

#define True 1
#define False 0





