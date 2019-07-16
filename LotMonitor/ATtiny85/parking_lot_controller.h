#pragma once

#include "tiny_ws2812.h"
#include "../common_communication_layer/communication_structs/common_structs.h"

#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

#define far		150
#define medium  100
#define close   30

#define leds_full	5
#define leds_empty	0

#define x1_first far
#define x2_first medium
#define y1_first leds_full
#define y2_first leds_empty

#define x1_second medium
#define x2_second close
#define y1_second leds_empty
#define y2_second leds_full





void parking_controller_poll(struct parking_status_t *parking_info, uint8_t is_busy);
