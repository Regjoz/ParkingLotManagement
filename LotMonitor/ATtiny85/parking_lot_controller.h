#pragma once

#include "tiny_ws2812.h"
#include "../common_communication_layer/communication_structs/common_structs.h"

#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif


void parking_controller_poll(struct parking_status_t *parking_info);