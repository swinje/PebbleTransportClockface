#include <pebble.h>
#include "persist.h"


// Config
// A struct for our specific settings 
ClaySettings settings;

int digital() {
   return(settings.digital);
}

char* blueFrom() {
  return(settings.blueFrom);
}
char* blueTo() {
  return(settings.blueTo);
}
char* redFrom() {
  return(settings.redFrom);
}
char* redTo() {
  return(settings.redTo);
}

// Initialize the default settings
void prv_default_settings() {
  strcpy(settings.blueFrom, "3012430");
  strcpy(settings.blueTo, "3010011");
  strcpy(settings.redFrom, "3010011");
  strcpy(settings.redTo, "3012430");
  settings.digital = 1;
}

// Read settings from persistent storage
void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void set_location(int l, const char* v) {
  switch(l) {
    case 1:
      snprintf(settings.blueFrom, sizeof(settings.blueFrom), "%s", v);
      break;
    case 2:
      snprintf(settings.blueTo, sizeof(settings.blueTo), "%s", v);
      break;
    case 3:
      snprintf(settings.redFrom, sizeof(settings.redFrom), "%s", v);
      break;
    case 4:
      snprintf(settings.redTo, sizeof(settings.redTo), "%s", v);
      break;
  }
  prv_save_settings();
}

bool set_digital(int d) {
  if (d == settings.digital)
      return false;
  settings.digital = d;
  prv_save_settings();
  return true;
}


