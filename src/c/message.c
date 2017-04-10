#include <pebble.h>
#include "message.h"
#include "analog.h"
#include "digital.h"
#include "persist.h"

// Messaging
static AppSync s_sync;
static uint8_t s_sync_buffer[256];

static int current_face;

void set_active_watchface(int wf) {
  current_face = wf;
}

// Error Callback for messaging
void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

// Callback for messaging
void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  
  static uint8_t direction = 0;
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "sync_tuple_changed_callback called with %i", (int) key);
  
  switch (key) {
    
    case DIRECTION_KEY:
      // App Sync keeps new_tuple in s_sync_buffer, so we may use it directly
      direction= new_tuple->value->uint8;
       break;   
    case DEPTIME_KEY:
      if (current_face == 0) {
        if (strlen(new_tuple->value->cstring)>=1) 
          set_analog_deptime(direction, atoi(new_tuple->value->cstring));
      } else {
        if (strlen(new_tuple->value->cstring)>=1) 
          set_digital_deptime(direction, atoi(new_tuple->value->cstring));
      }
      break;
    case WEATHER_ICON_KEY:
      if (current_face == 0) 
        switch_analog_icon(atoi(new_tuple->value->cstring));
      else
        switch_digital_icon(atoi(new_tuple->value->cstring));
      break;

    case WEATHER_TEMPERATURE_KEY:
      if (current_face == 0)
        set_analog_temp(new_tuple->value->cstring);
      else
        set_digital_temp(new_tuple->value->cstring);
      break;
    
    case BLUEFROM:
      set_location(1, new_tuple->value->cstring);
      break;
    case BLUETO:
      set_location(2, new_tuple->value->cstring);
      break;
    case REDFROM:
      set_location(3, new_tuple->value->cstring);
      break;
    case REDTO:
      set_location(4, new_tuple->value->cstring);
      break;
    case DIGITAL:
      set_digital(new_tuple->value->uint32);
      break;    
  }
}

// Send message to JS
void request_js(void) {
  DictionaryIterator *iter;
  AppMessageResult ar = app_message_outbox_begin(&iter);

  if (!iter) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "failed sending message %i", ar);
    // Error creating outbound message
    return;
  }

  // Write in values to send
  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();  
}

void init_message() {
  app_message_open(256, 256); 
  
  prv_load_settings();
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Digital Clock %d", digital());
  
   // Messaging values
  Tuplet initial_values[] = {
    TupletInteger(DIRECTION_KEY, (uint8_t) 1),      
    TupletCString(DEPTIME_KEY, "     "), 
    TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(WEATHER_TEMPERATURE_KEY, "      "),
    TupletCString(BLUEFROM, blueFrom()),
    TupletCString(BLUETO, blueTo()),
    TupletCString(REDFROM, redFrom()),
    TupletCString(REDTO, redTo()),
    TupletInteger(DIGITAL, digital()),
  };
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "app_sync_init");
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
  // End messaging
}

void deinit_message() {
  app_sync_deinit(&s_sync);
}

