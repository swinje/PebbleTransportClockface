#pragma once

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_MET1,
  RESOURCE_ID_IMAGE_MET2, 
  RESOURCE_ID_IMAGE_MET3, 
  RESOURCE_ID_IMAGE_MET4,
  RESOURCE_ID_IMAGE_MET5,
  RESOURCE_ID_IMAGE_MET6, 
  RESOURCE_ID_IMAGE_MET7,
  RESOURCE_ID_IMAGE_MET8,
  RESOURCE_ID_IMAGE_MET9, 
  RESOURCE_ID_IMAGE_MET10, 
  RESOURCE_ID_IMAGE_MET11,
  RESOURCE_ID_IMAGE_MET12,
  RESOURCE_ID_IMAGE_MET13, 
  RESOURCE_ID_IMAGE_MET14,
  RESOURCE_ID_IMAGE_MET15,
  RESOURCE_ID_IMAGE_MET20
};

enum MessageKey {
  DIRECTION_KEY = 0x0,      // TUPLE_INT
  DEPTIME_KEY = 0x1,        // TUPLE_CSTRING
  WEATHER_ICON_KEY = 0x2,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x3,  // TUPLE_CSTRING
  BLUEFROM = 0x4,         // TUPLE_CSTRING
  BLUETO = 0x5,  // TUPLE_CSTRING
  REDFROM = 0x6,         // TUPLE_CSTRING
  REDTO = 0x7,  // TUPLE_CSTRING
  DIGITAL = 0x8 // TUPLE_INT
};

//void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context);
//void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context);
void request_js(void);
void init_message();
void deinit_message();
void set_active_watchface(int wf);

