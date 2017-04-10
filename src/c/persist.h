#pragma once

#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {
  char blueFrom[10];
  char blueTo[10];
  char redFrom[10];
  char redTo[10];
  uint8_t  digital;
} __attribute__((__packed__)) ClaySettings;


void prv_default_settings();
void prv_load_settings();
void prv_save_settings();
void set_location(int l, const char* v);
int digital();
bool set_digital(int d);
char* blueFrom();
char* blueTo();
char* redFrom();
char* redTo();
