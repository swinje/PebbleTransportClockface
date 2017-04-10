#pragma once
#include "pebble.h"

void digital_init();
void digital_deinit();
void switch_digital_icon(int icon_index);
void set_digital_temp(const char *tmpstr);
void set_digital_deptime(int dir, int tm);